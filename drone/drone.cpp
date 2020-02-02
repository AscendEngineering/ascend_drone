#include "drone.h"

#include "ascend_zmq.h"
#include "constants.h"
#include "config_handler.h"
#include <iostream>
#include "dji_wrapper.h"



drone::drone(): context(1),send_socket(context, ZMQ_PUSH),recv_socket(context, ZMQ_PULL){

    //assign config vars
    drone_name = config_handler::instance()["name"];

    //set up comms
    recv_socket.bind("tcp://*:" + constants::to_drone);
    comm_items[0] = {static_cast<void*>(recv_socket),0,ZMQ_POLLIN,0};

    //set up dji osdk
    int functionTimeout = 1;

    // Setup OSDK.
    LinuxSetup linuxEnvironment(0,{});
    vehicle = linuxEnvironment.getVehicle();
    if (vehicle == NULL)
    {
        std::cout << "Vehicle not initialized, exiting.\n";

    }

    // Obtain Control Authority
    vehicle->obtainCtrlAuthority(functionTimeout);

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


void drone::test(){
    DJI::OSDK::Control::CtrlData command;
    command.flag &= 0xCF;
    command.flag |= VerticalLogic::VERTICAL_THRUST;
    command.x=1;
    command.y=1;
    command.z=1;
    command.yaw=0;
    vehicle->control->flightCtrl(CtrlData(command));
}


