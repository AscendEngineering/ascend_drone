#pragma once

/*
This file has all the operations that the drone can perform

*/


#include <string>
#include <vector>
#include <zmq.hpp>
#include <djiosdk/dji_vehicle.hpp>

class drone{

    public:
        drone();
        bool send_to_atc(std::string msg);
        std::vector<std::string> collect_messages();


    private:
        //vars
        std::string drone_name;
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