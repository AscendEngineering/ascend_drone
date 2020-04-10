#pragma once

/*
This file has all the operations that the drone can perform

*/


#include <string>
#include <vector>
#include <zmq.hpp>
#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/mavlink_passthrough/mavlink_passthrough.h>
#include "waypoints.h"
#include <memory>


class drone{

    public:
        drone();
        ~drone();

        /********* messaging *********/
        bool send_to_atc(std::string msg);
        std::vector<std::string> collect_messages();

        /********* flight controller *********/
        bool arm();
        bool takeoff(int altitude = 3);
        bool land();  /* DOES NOT BLOCK */
        bool kill();
        void manual();
        void test_motor(int motor = -1);

        /********* Waypoint Methods *********/
        bool start_mission(const waypoints& mission);
        bool pause_mission();
        bool cancel_mission();

        bool mission_finished();
        int current_mission_item();
        int total_mission_items();

        void wait_for_mission_completion();
        

    private:

        //px4
        bool connect_px4();

        //waypoints
        enum control_cmd{
            START,
            PAUSE,
            CANCEL
        };
        bool mission_control(control_cmd cmd);
        bool upload_waypoints(const std::vector<std::shared_ptr<mavsdk::MissionItem>>& waypoints);
        std::shared_ptr<Mission> m_mission;
        
        //vars
        std::string drone_name;
        mavsdk::Mavsdk px4;
        mavsdk::System* system;
        std::shared_ptr<mavsdk::Telemetry> telemetry;
        std::shared_ptr<mavsdk::Action> action;

        //coms
        zmq::context_t context;
        zmq::socket_t send_socket;
        zmq::socket_t recv_socket;
        zmq::pollitem_t comm_items [1];

        //disable copy and assign
        drone(const drone&);
        drone & operator=(const drone&);

        

};
