#pragma once

/*
This file has all the operations that the drone can perform

*/


#include <string>
#include <vector>
#include <zmq.hpp>
#include <djiosdk/dji_vehicle.hpp>
#include "dji_wrapper.h"

using namespace DJI::OSDK;
using namespace DJI::OSDK::Telemetry;

class drone{

    public:
        drone();
        ~drone();

        /********* messaging *********/
        bool send_to_atc(std::string msg);
        std::vector<std::string> collect_messages();


        /********* flight controller *********/
        void stop_motors(); //!!!!!DO NOT USE UNLESS DRONE IS ON GROUND!!!!!
        void test_motors();
        
        /********* sensor data *********/
        enum sensorData{
            LAT,        //double
            LNG,        //double
            ALT,        //double
            PITCH,      //int16_t
            ROLL,       //int16_t
            YAW,        //int16_t
            THROTTLE,   //int16_t
            VEL_X,      //double
            VEL_Y,      //double
            VEL_Z,      //double
        };

        template<class T>
        T get_sensor_data(sensorData sensor);
        int16_t getMotorSpeed(int motor_num);

    private:

        //functions
        bool drone_vehicle_init();

        //vars
        std::string drone_name;
        LinuxSetup linuxEnvironment;
        Vehicle* vehicle;

        //coms
        zmq::context_t context;
        zmq::socket_t send_socket;
        zmq::socket_t recv_socket;
        zmq::pollitem_t comm_items [1];

        //disable copy and assign
        drone(const drone&);
        drone & operator=(const drone&);

        

};


template<> int16_t drone::get_sensor_data(sensorData sensor);
template<> double drone::get_sensor_data(sensorData sensor);