#include "drone.h"

#include "ascend_zmq.h"
#include "constants.h"
#include "config_handler.h"
#include "drone_msg.h"
#include "manual_control.h"
#include "utilities.h"
#include <mavsdk/mavsdk.h>
#include <iostream>
#include <curses.h>
#include <thread> 
#include <chrono>  
#include <memory>
#include <future>
#include <mavsdk/plugins/offboard/offboard.h>
#include <mavsdk/plugins/calibration/calibration.h>
#include "package_control.h"

using namespace mavsdk;


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
    zmq_setsockopt(send_socket,ZMQ_LINGER,&linger_time, sizeof(linger_time));
    zmq_setsockopt(recv_socket,ZMQ_LINGER,&linger_time, sizeof(linger_time));

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

        //if battery is too low, 

        //send to atc
        bool sent = send_heartbeat(current_pos.longitude_deg,
            current_pos.latitude_deg,
            current_pos.relative_altitude_m, 
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

    std::this_thread::sleep_for(std::chrono::seconds(5));

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
	    std::cout << "Battery: "<< drone_sensors->get_battery() << std::endl;
            std::cout << "Next Operation: \n1)Takeoff \n2)Manual \n3)Magenet On \n4)Magnet Off \n5)Land \n6)Autonomous Land \n7)Exit" << std::endl;
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
                control_from_remote();
            }
            else if(user_resp=="7"){
                land();
                return;
            }
        }

        //output current position
        auto ground_pos = drone_sensors->get_position();
        std::cout << "absolute pos: " << ground_pos.absolute_altitude_m << std::endl;
        std::cout << "relative pos: " << ground_pos.relative_altitude_m  << std::endl;
    }
}
void drone::control_from_remote(){

    //enter manual mode
    auto offboard = std::make_shared<Offboard>(*system);
    offboard->set_velocity_body({0, 0, 0, 0}); /* Needed */
    Offboard::Result offboard_result = offboard->start();

    //error check
    if(offboard_result != Offboard::Result::Success){
        std::cerr << "Error gaining offboard control" << std::endl;
        return;
    }


    //enable killswitch
    utilities::line_buffer(false);

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
                remote_controlling = false;
                break;
            }
            else if(cmd_msg.has_offset()){
                ascend::move_offset landing_cmd =  cmd_msg.offset();

                float x = landing_cmd.x();
                float y = landing_cmd.y();
                float z = landing_cmd.z();
                float yaw = 0;
                float rate = landing_cmd.rate();

                if(landing_cmd.has_yaw()){
                    yaw = landing_cmd.yaw();
                }
                
                std::cout << "Command-> X:" << x << " Y:"<< y << " Z:" << z << " Yaw:" << yaw << " Rate:" << rate << std::endl;
                std::cout << "Height: " << drone_sensors->get_position().relative_altitude_m << std::endl;
                offboard->set_velocity_body({y*rate, x*rate, z*rate, yaw*YAW_FACTOR*rate});
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
                else if(cmd==ascend::PICKUP_PACKAGE){
                    package_control::get_instance().pickup();
                }
                else if(cmd==ascend::DROPOFF_PACKAGE){
                    package_control::get_instance().release();
                }

                //retart offboard
                offboard->set_velocity_body({0, 0, 0, 0}); /* Needed */
                Offboard::Result offboard_result = offboard->start();
                if(offboard_result != Offboard::Result::Success){
                    std::cerr << "Error gaining offboard control" << std::endl;
                    return;
                }
                utilities::line_buffer(false);

            }
        }

        //heartbeat
        auto current_time = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_time = current_time-last_heartbeat;
        if(elapsed_time.count() > 0.5){
            send_heartbeat();
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
    land();
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
            std::cout << prog.status_text;
        }
        if(prog.progress == 1){
            calibration_complete = true;
        }
        std::cout << std::endl;
    };

    /* CALIBRATION BEGIN */
    std::shared_ptr<Calibration> calibration_engine = std::make_shared<Calibration>(*system);

    //gyro calibration
    if(sensor == 1 || sensor == -1){
        std::cout << "Calibrating Gyro..." << std::endl;
        calibration_engine->calibrate_gyro_async(calibration_info);
        while(!calibration_complete){}
        calibration_complete = false;
        std::this_thread::sleep_for(std::chrono::seconds(10));
        std::cout << "...Gyro Calibrated" << std::endl;
        
    }
    
    //level_horizon calibration
    if(sensor == 2 || sensor == -1){
        std::cout << "Calibrating level_horizon..." << std::endl;
        calibration_engine->calibrate_level_horizon_async(calibration_info);
        while(!calibration_complete){}
        calibration_complete = false;
        std::this_thread::sleep_for(std::chrono::seconds(10));
        std::cout << "...level_horizon Calibrated" << std::endl;
    }

    //accelerometer calibration
    if(sensor == 3 || sensor == -1){
        std::cout << "Calibrating accelerometer..." << std::endl;
        calibration_engine->calibrate_accelerometer_async(calibration_info);
        while(!calibration_complete){}
        calibration_complete = false;
        std::this_thread::sleep_for(std::chrono::seconds(10));
        std::cout << "...accelerometer calibrated" << std::endl;
    }

    //magnetometer calibration
    if(sensor == 4 || sensor == -1){
        std::cout << "Calibrating magnetometer..." << std::endl;
        calibration_engine->calibrate_magnetometer_async(calibration_info);
        while(!calibration_complete){}
        calibration_complete = false;
        std::this_thread::sleep_for(std::chrono::seconds(10));
        std::cout << "...magnetometer calibrated" << std::endl;
    }

    /* CALIBRATION END */
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
        std::cout << progress_percentage << std::endl;    
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

    std::cout << "Connecting" << std::endl;
    while (!px4.is_connected()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout<<".";
    }

    //assigning the system
    system = &px4.system();
    telemetry = std::make_shared<Telemetry>(*system);
    action = std::make_shared<Action>(*system);
    drone_sensors = std::make_shared<sensors>(telemetry);

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
