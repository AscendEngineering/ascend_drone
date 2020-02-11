#include "drone.h"

#include "ascend_zmq.h"
#include "constants.h"
#include "config_handler.h"
#include <iostream>

//temp
#include <thread> 
#include <chrono>  

using namespace DJI::OSDK;

drone::drone(): context(1),
                send_socket(context, ZMQ_PUSH),
                recv_socket(context, ZMQ_PULL),
                linuxEnvironment(0, {}){

    //assign config vars
    drone_name = config_handler::instance()["name"];

    //set up comms
    recv_socket.bind("tcp://*:" + constants::to_drone);
    comm_items[0] = {static_cast<void*>(recv_socket),0,ZMQ_POLLIN,0};

    //set up drone vehicle
    bool connection_success = drone_vehicle_init();

}

drone::~drone(){
    // TODO: remove the packages we were subscribed to here
    int responseTimeout = 1;
    vehicle->subscribe->removePackage(0, responseTimeout);
    vehicle->subscribe->removePackage(1, responseTimeout);
    vehicle->subscribe->removePackage(2, responseTimeout);
    vehicle->subscribe->removePackage(3, responseTimeout);

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

void drone::stop_motors(){
    vehicle->control->disArmMotors(5);
}

void drone::test_motors(){
    
    std::cout<<"Starting the motor"<< std::endl;
    vehicle->control->armMotors(5);
    
    //sleep
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));

    std::cout<<"Stoping the motor"<< std::endl;
    vehicle->control->disArmMotors(5);

}

void drone::spin_at_percentage(float percentage){

    //vehicle->control->armMotors(5);

    std::cout << "Motors: " << percentage << "%" << std::endl;
    Control::CtrlData newCmd(
        DJI::OSDK::Control::VerticalLogic::VERTICAL_THRUST,0,0,percentage,0);

    vehicle->control->flightCtrl(newCmd);

    //sleep
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    stop_motors();

}

template<>
int16_t drone::get_sensor_data(sensorData sensor){

    int16_t retval;
    auto& drone_data = vehicle->subscribe;
    
    switch(sensor){
    
        case(PITCH):{
            retval = drone_data->getValue<TOPIC_RC>().pitch;
            break;
        }
        case(ROLL):{
            retval = drone_data->getValue<TOPIC_RC>().roll;
            break;
        }
        case(YAW):{
            retval = drone_data->getValue<TOPIC_RC>().yaw;
            break;
        }
        case(THROTTLE):{
            retval = drone_data->getValue<TOPIC_RC>().throttle;
            break;
        }
        default:{
            //TODO throw exception
            break;
        }
    }

    return retval;
}

template<>
double drone::get_sensor_data(sensorData sensor){

    double retval;
    auto& drone_data = vehicle->subscribe;

    switch(sensor){
        case(LAT):{
            retval = drone_data->getValue<TOPIC_GPS_FUSED>().latitude;
            break;
        }
        case(LNG):{
            retval = drone_data->getValue<TOPIC_GPS_FUSED>().longitude;
            break;
        }
        case(ALT):{
            std::cout << "Actual: " << drone_data->getValue<TOPIC_ALTITUDE_FUSIONED>() << std::endl;
            retval = (double) drone_data->getValue<TOPIC_ALTITUDE_FUSIONED>();
            break;
        }
        case(VEL_X):{
            retval = (double) drone_data->getValue<TOPIC_VELOCITY>().data.x;
            break;
        }
        case(VEL_Y):{
            retval = (double) drone_data->getValue<TOPIC_VELOCITY>().data.y;
            break;
        }
        case(VEL_Z):{
            retval = (double) drone_data->getValue<TOPIC_VELOCITY>().data.z;
            break;
        }
        default:{
            //TODO throw exception
            break;
        }
    }
    return retval; 
}


bool drone::drone_vehicle_init(){
    
    //set up dji osdk
    int functionTimeout = 1;

    vehicle = linuxEnvironment.getVehicle();
    if (vehicle == NULL)
    {
        std::cout << "Vehicle not initialized.\n";
        return false;
    }

    // Obtain Control Authority
    vehicle->obtainCtrlAuthority(functionTimeout);

    //subscribe to sensor data
    bool succ = subscribeToData(vehicle);
    if(!succ){
        return false;
    }

    return true;
}


