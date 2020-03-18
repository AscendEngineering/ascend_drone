#include "drone.h"

#include "ascend_zmq.h"
#include "constants.h"
#include "config_handler.h"
#include "manual_control.h"

#include <mavsdk/mavsdk.h>
#include <iostream>
#include <curses.h>
#include <thread> 
#include <chrono>  
#include <memory>

using namespace mavsdk;

drone::drone(): context(1),
                send_socket(context, ZMQ_PUSH),
                recv_socket(context, ZMQ_PULL)
                {

    //assign config vars
    drone_name = config_handler::instance()["name"];

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


bool drone::arm(){

    //check our health
    std::cout << "Checking Drone Health..." << std::endl;
    while (telemetry->health_all_ok() != true) {
        Telemetry::Health health = telemetry->health();
        std::cout << health << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "...Drone is healthy" << std::endl;

    //arm
    std::cout << "Arming..." << std::endl;
    const Action::Result arm_result = action->arm();

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


void drone::test_motor(int motor){

    //going to have to take a passthrough command   
    

}