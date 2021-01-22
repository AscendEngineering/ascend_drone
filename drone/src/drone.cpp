#include "drone.h"

#include "april_land.h"
#include "ascend_zmq.h"
#include "constants.h"
#include "config_handler.h"
#include "drone_msg.h"
#include "loguru.hpp"
#include "manual_control.h"
#include "package_control.h"
#include "utilities.h"

#include <mavsdk/mavsdk.h>
#include <iostream>
#include <curses.h>
#include <thread> 
#include <chrono>  
#include <memory>
#include <future>
#include <math.h>
#include <mavsdk/plugins/calibration/calibration.h>
#include <mavsdk/plugins/log_files/log_files.h>
#include <stdio.h>


using namespace mavsdk;


namespace {
    float adjust_yaw(float in_yaw, float rate){
        if(in_yaw==0 || rate ==0){
            return 0;
        }
        return in_yaw * (18+ (1/rate));
    }
}


drone::drone(bool in_simulation): context(1),
                send_socket(context, ZMQ_PUSH),
                recv_socket(context, ZMQ_PULL),
                simulation(in_simulation)
                {
 
    load_config_vars();

    //set up comms
    recv_socket.bind("tcp://*:" + constants::to_drone);
    comm_items[0] = {static_cast<void*>(recv_socket),0,ZMQ_POLLIN,0};
    int linger_time = 1000;
    zmq_setsockopt(&send_socket,ZMQ_LINGER,&linger_time, sizeof(linger_time));
    zmq_setsockopt(&recv_socket,ZMQ_LINGER,&linger_time, sizeof(linger_time));

    //set up drone vehicle
    connect_px4();

    //register with ATC
    register_with_atc();
}

drone::~drone(){

    land();

    //make sure drone is on ground
}


bool drone::send_to_atc(std::string msg){
    //send
    return comm::send_msg(send_socket,drone_name,msg,atc_ip);
}

bool drone::send_ack(){
    
    return comm::send_ack(send_socket,
        drone_name,
        utilities::resolveDNS(atc_ip));
}


std::vector<std::string> drone::collect_messages(){
    std::vector<std::string> messages;

    //while true
    while(true){

        //poll
        zmq::poll(&comm_items[0], 1, 1);

        //if messages, collect
        if(comm_items[0].revents & ZMQ_POLLIN){
            // std::string sender;
            // std::string operation;
            std::string data;


            // comm::get_msg_header(recv_socket,sender,operation);
            
            // //acknowledgement TODO
            // if(operation=="A"){
            //     std::cout<<"ATC Acknowledged"<<std::endl;
            //     continue;
            // }

            data = comm::get_msg_data(recv_socket);
            //send_ack();
            messages.push_back(data);
        }
        else{
            return messages;
        }
    }
}

bool drone::register_with_atc(){

    //craft proto message to register
    return true;

}

void drone::send_heartbeat(){
        //collect sensor info (lat,lng,alt,battery)
        position current_pos = drone_sensors->get_position();
        float current_battery = drone_sensors->get_battery();
        
        double sending_height = current_pos.relative_altitude_m;

        //check height sensors
        if(ULTRA_ENABLED){
            int ultra_height = sensor_group.get_ultrasonic_distance();
            if(ultra_height < 100){
                sending_height = (double)ultra_height/(double)100;
            }
        }
        else if(TFMINI_ENABLED){
            distance tfmini_reading= drone_sensors->get_distance();
            bool in_range = !std::isnan(tfmini_reading.current_distance_m)
                && tfmini_reading.current_distance_m > tfmini_reading.minimum_distance_m
                && tfmini_reading.current_distance_m < tfmini_reading.maximum_distance_m;
            if(in_range){
                sending_height = (double)tfmini_reading.current_distance_m / (double)100;
            }
        }

        //send to atc
        LOG_S(INFO) << "\tlng: " << current_pos.longitude_deg 
            << " lat: " << current_pos.latitude_deg
            << " alt: " << sending_height;

        bool sent = send_heartbeat(current_pos.longitude_deg,
            current_pos.latitude_deg,
            sending_height, 
            current_battery);

}

bool drone::unregister_with_atc(){
    return true;
}

bool drone::arm(){

    //check our health
    while (telemetry->health_all_ok() != true) {
        Telemetry::Health health = telemetry->health();
        std::cerr << "Drone is not healthy: " << health << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    //arm
    const Action::Result arm_result = action->arm();

    if(arm_result != Action::Result::Success){
        return false;
    }
    else{
        return true;
    }
}

bool drone::takeoff(int altitude /* = 3 */){

    //check drone is armed
    if(!(telemetry->armed())){
        std::cerr << "DRONE IS NOT ARMED, TAKEOFF REJECTED..." << std::endl;
        return false;
    }

    //takeoff
    action->set_takeoff_altitude(altitude);
    const Action::Result takeoff_result = action->takeoff();
    if (takeoff_result != Action::Result::Success) {
        return false;
    }

    return true;    
}

bool drone::land(){
    /* DOES NOT BLOCK */

    const Action::Result land_result = action->land();
    if(land_result != Action::Result::Success){
        return false;
    }
    return true;
}

bool drone::kill(){

    const Action::Result land_result = action->kill();
    if(land_result != Action::Result::Success){
        return false;
    }
    return true;
}

void drone::manual(){

    bool operating = true;
    while(operating){

        //get next operation
        bool correct_resp = false;
        while(!correct_resp){

            std::string user_resp;
	        LOG_S(INFO) << "Battery: "<< drone_sensors->get_battery();
            std::cerr << "Next Operation: \n1)Takeoff \n2)Manual \n3)Magenet On \n4)Magnet Off \n5)Land \n6)Smart Land \n7)April Land \n8)Remote Control \n9)Exit" << std::endl;
            std::cin >> user_resp;

            if(user_resp == "1"){
                correct_resp = true;
                arm();
                std::this_thread::sleep_for(std::chrono::seconds(1));
                takeoff();
            }
            else if(user_resp=="2"){
                manual_control drone_control(system);
            }
            else if(user_resp=="3"){
                package_control::get_instance().pickup();
            }
            else if(user_resp=="4"){
                package_control::get_instance().release();
            }
            else if(user_resp=="5"){
               	land();
            }
            else if(user_resp=="6"){
                std::string send_msg = msg_generator::generate_land_request(drone_name);
                comm::send_msg(send_socket,drone_name,send_msg,atc_ip);
                control_from_remote(true);
            }
            else if(user_resp=="7"){
                //enter manual mode
                auto offboard = std::make_shared<Offboard>(*system);
                offboard->set_velocity_body({0, 0, 0, 0}); /* Needed */
                Offboard::Result offboard_result = offboard->start();

                //error check
                if(offboard_result != Offboard::Result::Success){
                    std::cerr << "Error gaining offboard control" << std::endl;
                    return;
                }

                //april
                LOG_S(INFO) << "Activating April Assist";
                april_land lander;
                bool above_april = lander.execute(offboard,drone_sensors,simulation);
                offboard->set_velocity_body({0, 0, 0, 0}); /* Needed */
                offboard_result = offboard->start();
                if(offboard_result != Offboard::Result::Success){
                    std::cerr << "Error gaining offboard control" << std::endl;
                    return;
                }
                //land();
            }
            else if(user_resp=="8"){
                std::string send_msg = msg_generator::generate_land_request(drone_name);
                comm::send_msg(send_socket,drone_name,send_msg,atc_ip);
                control_from_remote();
            }
            else if(user_resp=="9"){
                land();
                return;
            }
        }

        //output current position
        auto ground_pos = drone_sensors->get_position();
        LOG_S(INFO) << "absolute pos: " << ground_pos.absolute_altitude_m;
        LOG_S(INFO) << "relative pos: " << ground_pos.relative_altitude_m;    
    }
}

void drone::control_from_remote(bool april_assist){

    //enter manual mode
    auto offboard = std::make_shared<Offboard>(*system);
    offboard->set_velocity_body({0, 0, 0, 0}); /* Needed */
    Offboard::Result offboard_result = offboard->start();

    //error check
    if(offboard_result != Offboard::Result::Success){
        std::cerr << "Error gaining offboard control" << std::endl;
        return;
    }

    utilities::line_buffer(false);
    drone::collect_messages(); //throwaway builtup messages

    auto last_heartbeat = std::chrono::system_clock::now();

    //control
    bool remote_controlling = true;
    while(remote_controlling){

        //check for killswitch
        std::string cmd_line = utilities::get_term_input();
        if(cmd_line.find('x') != std::string::npos){
            break;
        }

        //process messages
        std::vector<std::string> messages = drone::collect_messages();
        for(auto msg: messages){
            ascend::msg cmd_msg = msg_generator::deserialize(msg);

            if(cmd_msg.has_stop_remote()){
                LOG_S(INFO) << "Stopping remote connection";
                remote_controlling = false;
                if(april_assist){
                    LOG_S(INFO) << "Activating April Assist";
                    april_land lander;
                    std::cout << "sim: " << simulation << std::endl;
                    bool above_april = lander.execute(offboard,drone_sensors,simulation);
                    if(above_april){
                        LOG_S(INFO) << "Killing Motors";
                        land();
                    }
                    else{
                        LOG_S(INFO) << "April Failed";
                    }
                }
                break;
            }
            else if(cmd_msg.has_offset()){
                ascend::move_offset landing_cmd =  cmd_msg.offset();

                //reinitiate offboard if needed
                if(!offboard->is_active()){
                    offboard->set_velocity_body({0, 0, 0, 0}); /* Needed */
                    Offboard::Result offboard_result = offboard->start();
                    if(offboard_result != Offboard::Result::Success){
                        std::cerr << "Error gaining offboard control" << std::endl;
                        return;
                    }
                }

                float x = landing_cmd.x();
                float y = landing_cmd.y();
                float z = landing_cmd.z();
                float yaw = landing_cmd.yaw();
                float rate = landing_cmd.rate();

                //send cmd
                LOG_S(INFO) << "Command-> X:" << x << " Y:"<< y << " Z:" << z << " Yaw:" << yaw << " Rate:" << rate << std::endl;
                offboard->set_velocity_body({y*rate, x*rate, z*rate, ::adjust_yaw(yaw,rate)});
            }
            else if(cmd_msg.has_action_cmd()){
                
                //stop offboard
                offboard->set_velocity_body({0, 0, 0, 0}); /* Needed */
                offboard_result = offboard->stop();
                utilities::line_buffer(true);
                if(offboard_result != Offboard::Result::Success){
                    std::cerr << "Error stopping offboard control" << std::endl;
                }

                //issue command
                ascend::action_cmd_enum cmd = cmd_msg.action_cmd().cmd();
                if(cmd==ascend::TAKEOFF){
                    takeoff();
                }
                else if(cmd==ascend::LAND){
                    land();
                }
                else if(cmd==ascend::PICKUP){
                    package_control::get_instance().pickup();
                }
                else if(cmd==ascend::DROPOFF){
                    package_control::get_instance().release();
                }
                else if(cmd==ascend::KILL){
                    kill();
                }
                else if(cmd==ascend::ARM){
                    arm();
                }

                utilities::line_buffer(false);
            }
        }

        //heartbeat
        auto current_time = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_time = current_time-last_heartbeat;
        if(elapsed_time.count() > 0.5){
            // velocity curr_vel = drone_sensors->get_velocity();
            // LOG_S(INFO) << "north_m_s: " << curr_vel.north_m_s << " east_m_s: " << curr_vel.east_m_s << " down_m_s: " << curr_vel.down_m_s << std::endl;
            // send_heartbeat();
            last_heartbeat = std::chrono::system_clock::now();
        }
    }

    //stop offboard
    offboard->set_velocity_body({0, 0, 0, 0}); /* Needed */
    offboard_result = offboard->stop();

    //stop killswitch
    utilities::line_buffer(true);

    //error check
    if(offboard_result != Offboard::Result::Success){
        std::cerr << "Error stopping offboard control" << std::endl;
    }

    //make sure ffmpeg is closed
    FILE* kill_ffmpeg = popen("pkill -f ffmpeg","r");
}


void drone::test_motor(int motor){

    //setup
    std::shared_ptr<mavsdk::Shell> shell = std::make_shared<Shell>(*system);
    shell->subscribe_receive([](const std::string output) {});

    //spin all motors
    std::string motors = "";
    if(motor == -1){
        motors = "12345678";
    }
    else{
        motors = std::to_string(motor);
    }

    //send command
    std::string command = "pwm test -c " + motors + " -p 1000";
    mavsdk::Shell::Result result = shell->send(command);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    result = shell->send("c");
}

void drone::calibrate(int sensor){

    bool calibration_complete = false;

    //define callback function
    std::function<void(Calibration::Result, Calibration::ProgressData)> calibration_info = [&](Calibration::Result res, Calibration::ProgressData prog){
        if(prog.has_status_text){
            std::cerr << prog.status_text;
        }
        if(prog.progress == 1){
            calibration_complete = true;
        }
        std::cerr << std::endl;
    };

    /* CALIBRATION BEGIN */
    std::shared_ptr<Calibration> calibration_engine = std::make_shared<Calibration>(*system);

    //gyro calibration
    if(sensor == 1 || sensor == -1){
        std::cerr << "Calibrating Gyro..." << std::endl;
        calibration_engine->calibrate_gyro_async(calibration_info);
        while(!calibration_complete){}
        calibration_complete = false;
        std::this_thread::sleep_for(std::chrono::seconds(10));
        std::cerr << "...Gyro Calibrated" << std::endl;
        
    }
    
    //level_horizon calibration
    if(sensor == 2 || sensor == -1){
        std::cerr << "Calibrating level_horizon..." << std::endl;
        calibration_engine->calibrate_level_horizon_async(calibration_info);
        while(!calibration_complete){}
        calibration_complete = false;
        std::this_thread::sleep_for(std::chrono::seconds(10));
        std::cerr << "...level_horizon Calibrated" << std::endl;
    }

    //accelerometer calibration
    if(sensor == 3 || sensor == -1){
        std::cerr << "Calibrating accelerometer..." << std::endl;
        calibration_engine->calibrate_accelerometer_async(calibration_info);
        while(!calibration_complete){}
        calibration_complete = false;
        std::this_thread::sleep_for(std::chrono::seconds(10));
        std::cerr << "...accelerometer calibrated" << std::endl;
    }

    //magnetometer calibration
    if(sensor == 4 || sensor == -1){
        std::cerr << "Calibrating magnetometer..." << std::endl;
        calibration_engine->calibrate_magnetometer_async(calibration_info);
        while(!calibration_complete){}
        calibration_complete = false;
        std::this_thread::sleep_for(std::chrono::seconds(10));
        std::cerr << "...magnetometer calibrated" << std::endl;
    }

    /* CALIBRATION END */
}

void drone::get_px4log(){
    auto log_files = std::make_shared<LogFiles>(*system);
    std::pair< LogFiles::Result, std::vector< LogFiles::Entry > > log_result = log_files->get_entries();
    std::cout << "Number of log files: " << log_result.second.size() << std::endl;
}

bool drone::start_mission(const waypoints& mission){

    //form waypoint if null
    if(m_mission == nullptr){
        m_mission = std::make_shared<Mission>(*system);
    }
    
    //upload the given mission
    mavsdk::Mission::MissionPlan mission_plan;
    bool succ_upload = upload_waypoints(mission_plan);

    if(!succ_upload){
        return false;
    }

    return mission_control(START);
}

bool drone::pause_mission(){
    return mission_control(PAUSE);
}

bool drone::cancel_mission(){
    return mission_control(CANCEL);
}

bool drone::mission_finished(){
    return m_mission->is_mission_finished().second;
}

int drone::current_mission_item(){
    return m_mission->mission_progress().current;
}

int drone::total_mission_items(){
    return m_mission->mission_progress().total;
}

void drone::wait_for_mission_completion(){

    if(m_mission == nullptr){
        return;
    }

    //setup variables
    bool finished = false;
    float progress = 0;

    //subscribe to the updates 
    m_mission->subscribe_mission_progress([&](mavsdk::Mission::MissionProgress progress) { 
        int progress_percentage = (float)(progress.current/progress.total); 
        LOG_S(INFO) << progress_percentage << std::endl;    
    });

    //wait until done
    while(progress != 1){}
}

bool drone::connect_px4(){

    if(!simulation){
        ConnectionResult conn_result = px4.add_serial_connection("/dev/ttyS0");
    }
    else{
        ConnectionResult conn_result = px4.add_udp_connection("",14550);
    }

    LOG_S(INFO) << "Connecting" << std::endl;
    while (!px4.is_connected()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        LOG_S(INFO)<<".";
    }

    //assigning the system
    system = &px4.system();
    telemetry = std::make_shared<Telemetry>(*system);
    action = std::make_shared<Action>(*system);
    drone_sensors = std::make_shared<px4_sensors>(telemetry);

    return true;
}

bool drone::mission_control(control_cmd cmd){

    if(m_mission == nullptr){
        return false;
    }
    
    auto prom = std::make_shared<std::promise<Mission::Result>>();
    auto future_result = prom->get_future();

    switch(cmd){
        case START:{
            m_mission->start_mission_async([prom](Mission::Result result) { prom->set_value(result);});
            break;
        }
        case PAUSE:{
            m_mission->pause_mission_async([prom](Mission::Result result) {prom->set_value(result);});
            break;
        }
        case CANCEL:{
            m_mission->clear_mission_async([prom](Mission::Result result) {prom->set_value(result);});
            break;
        }
        default:{
            std::cerr << "Invalid command" << std::endl;
            return false;
        }

    }
    
    const Mission::Result result = future_result.get();
    return (result==Mission::Result::Success);
}

bool drone::upload_waypoints(const mavsdk::Mission::MissionPlan& mission_plan){

    if(m_mission == nullptr){
        return false;
    }

    //setup promise/future
    auto prom = std::make_shared<std::promise<Mission::Result>>();
    auto future_result = prom->get_future();

    //for each waypoint
    m_mission->upload_mission_async(mission_plan, 
        [prom](Mission::Result result) { prom->set_value(result);});
    const Mission::Result result = future_result.get();
    
    if(result==Mission::Result::Success){
        return true;
    }
    else{
        return false;
    }
}

void drone::load_config_vars(){

    //drone name
    drone_name = config_handler::instance()["drone_name"];
    if(std::getenv("LOCAL_NETWORK") == nullptr){
        atc_ip = config_handler::instance()["atc_ip"];
    }
    else{
        atc_ip = "10.0.0.44"; //atc ip on local network
    }
    atc_ip = atc_ip + ":" + constants::from_drone;

}

bool drone::send_heartbeat(double lng, double lat, double alt, double bat_percentage){
    std::string msg = msg_generator::generate_heartbeat(drone_name, lng, lat, alt, bat_percentage);
    return comm::send_msg(send_socket,drone_name,msg,atc_ip);
}
