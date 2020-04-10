#include "drone.h"

#include "ascend_zmq.h"
#include "constants.h"
#include "config_handler.h"
#include "drone_msg.h"
#include "manual_control.h"

#include <mavsdk/mavsdk.h>
#include <iostream>
#include <curses.h>
#include <thread> 
#include <chrono>  
#include <memory>
#include <future>

using namespace mavsdk;

drone::drone(): context(1),
                send_socket(context, ZMQ_PUSH),
                recv_socket(context, ZMQ_PULL)
                {
 
    //assign config vars
    drone_name = config_handler::instance()["drone_name"];

    //set up comms
    recv_socket.bind("tcp://*:" + constants::to_drone);
    comm_items[0] = {static_cast<void*>(recv_socket),0,ZMQ_POLLIN,0};

    //set up drone vehicle
    connect_px4();
}

drone::~drone(){

    land();

    //make sure drone is on ground
}


bool drone::send_to_atc(std::string msg){

    //read atc_ip and name from config
    std::string atc_ip = config_handler::instance()["atc_ip"] + constants::from_drone;

    //send
    return comm::send_msg(send_socket,drone_name,msg,atc_ip);
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
            comm::send_ack(send_socket,drone_name,"tcp://localhost:" + constants::from_drone);
            messages.push_back(data);
        }
        else{
            return messages;
        }
    }
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

    if(arm_result != Action::Result::SUCCESS){
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
    if (takeoff_result != Action::Result::SUCCESS) {
        return false;
    }

    return true;    
}

bool drone::land(){
    /* DOES NOT BLOCK */

    const Action::Result land_result = action->land();
    if(land_result != Action::Result::SUCCESS){
        return false;
    }
    return true;
}

bool drone::kill(){

    const Action::Result land_result = action->kill();
    if(land_result != Action::Result::SUCCESS){
        return false;
    }
    return true;
}

void drone::manual(){
    manual_control drone_control(system);
}

bool drone::start_mission(const waypoints& mission){

    //form waypoint if null
    if(m_mission == nullptr){
        m_mission = std::make_shared<Mission>(*system);
    }
    
    //upload the given mission
    bool succ_upload = upload_waypoints(mission.get_waypoints());

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
    return m_mission->mission_finished();
}

int drone::current_mission_item(){
    return m_mission->current_mission_item();
}

int drone::total_mission_items(){
    return m_mission->total_mission_items();
}

void drone::wait_for_mission_completion(){

    if(m_mission == nullptr){
        return;
    }

    //setup variables
    bool finished = false;
    auto prom = std::make_shared<std::promise<float>>();
    auto future_result = prom->get_future();

    //subscribe to the updates 
    m_mission->subscribe_progress([prom](int current, int total) { prom->set_value((float)(current/total));});

    //wait until reached last waypoint
    while(!finished){
        float result = future_result.get();
        if(result == 1.0){
            finished = true;
        }
    }
}

bool drone::connect_px4(){

    ConnectionResult conn_result = px4.add_serial_connection("/dev/ttyS0");

    std::cout << "Connecting";
    while (!px4.is_connected()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout<<".";
    }

    //assigning the system
    system = &px4.system();
    telemetry = std::make_shared<Telemetry>(*system);
    action = std::make_shared<Action>(*system);

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
    return (result==Mission::Result::SUCCESS);
}

bool drone::upload_waypoints(const std::vector<std::shared_ptr<mavsdk::MissionItem>>& waypoints){

    if(m_mission == nullptr){
        return false;
    }

    //setup promise/future
    auto prom = std::make_shared<std::promise<Mission::Result>>();
    auto future_result = prom->get_future();

    //for each waypoint
    m_mission->upload_mission_async(waypoints, [prom](Mission::Result result) { prom->set_value(result); });
    const Mission::Result result = future_result.get();
    
    if(result==Mission::Result::SUCCESS){
        return true;
    }
    else{
        return false;
    }
}


void drone::test_motor(int motor){

    std::shared_ptr<mavsdk::Shell> shell = std::make_shared<Shell>(*system);

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
    shell->shell_command({true,5000,command.c_str()});

    //wait
    std::this_thread::sleep_for(std::chrono::seconds(5));

    //kill motors
    shell->shell_command({true,5000,"c"});

}