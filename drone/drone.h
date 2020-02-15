#pragma once

/*
This file has all the operations that the drone can perform

*/


#include <string>
#include <vector>
#include <zmq.hpp>

class drone{

    public:
        drone();
        bool send_to_atc(std::string msg);
        std::vector<std::string> collect_messages();
        
        void set_waypoints(double long_in, double lat_in, double alt_in);
        


    private:
        //vars
        std::string drone_name;
        std::double long_waypoint;
        std::double lat_waypoint;
        std::double alt_waypoint;

        //coms
        zmq::context_t context;
        zmq::socket_t send_socket;
        zmq::socket_t recv_socket;
        zmq::pollitem_t comm_items [1];

        //disable copy and assign
        drone(const drone&);
        drone & operator=(const drone&);

};
