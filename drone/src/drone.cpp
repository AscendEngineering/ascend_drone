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
    //register_with_atc();

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
            std::string sender;
            std::string operation;
            std::string data;

            comm::get_msg_header(recv_socket,sender,operation);
            
            //acknowledgement TODO
            if(operation=="A"){
                std::cout<<"ATC Acknowledged"<<std::endl;
                continue;
            }

            data = comm::get_msg_data(recv_socket);
            send_ack();
            messages.push_back(data);
        }
        else{
            return messages;
        }
    }
}

bool drone::register_with_atc(){

    //craft proto message to register
    std::string msg = msg_generator::generate_status_change(drone_name,drone_status::AVAILABLE);
    return comm::send_msg(send_socket,drone_name,msg,atc_ip);
}

void drone::send_heartbeat(){
        //collect sensor info (lat,lng,alt,battery)
        position current_pos = drone_sensors->get_position();
        float current_battery = drone_sensors->get_battery();

        //send to atc
        send_heartbeat(current_pos.longitude_deg,
            current_pos.latitude_deg,
            current_pos.absolute_altitude_m, 
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
            std::cout << "Next Operation: \n1)Takeoff \n2)Magenet On \n3)Magnet Off \n4)Autonomous Land \n5)Exit" << std::endl;
            std::cin >> user_resp;

            if(user_resp == "1"){
                correct_resp = true;
                arm();
                std::this_thread::sleep_for(std::chrono::seconds(1));
                takeoff();
                std::this_thread::sleep_for(std::chrono::seconds(10));
                manual_control drone_control(system);
            }
            else if(user_resp=="2"){
                package_control::get_instance().pickup();
            }
            else if(user_resp=="3"){
                package_control::get_instance().release();
            }
            else if(user_resp=="4"){
                
            }
            else if(user_resp=="5"){
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


void drone::test_motor(int motor){

    //setup
    std::shared_ptr<mavsdk::Shell> shell = std::make_shared<Shell>(*system);
    shell->subscribe_receive([](const std::string output) {});

    //spin all motors
    std::string motors = "";
    if(motor == -1){
        motors = "123456";
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

bool drone::send_heartbeat(int lng, int lat, int alt, int bat_percentage){
    std::string msg = msg_generator::generate_heartbeat(drone_name, lng, lat, alt, bat_percentage);
    return comm::send_msg(send_socket,drone_name,msg,atc_ip);
}
