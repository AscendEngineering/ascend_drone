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
#include <mavsdk/plugins/shell/shell.h>
#include "waypoints.h"
#include <memory>
#include "sensors.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>



class drone{

    public:
        drone(bool in_simulation=false);
        ~drone();

        /********* messaging *********/
        bool send_to_atc(std::string msg);
        bool send_ack();
        std::vector<std::string> collect_messages();

        /************* registration ***************/
        bool register_with_atc();
        void send_heartbeat();
        
        bool unregister_with_atc();

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
        std::string atc_ip;
        mavsdk::Mavsdk px4;
        mavsdk::System* system;
        std::shared_ptr<mavsdk::Telemetry> telemetry;
        std::shared_ptr<mavsdk::Action> action;
        std::shared_ptr<sensors> drone_sensors;
        std::shared_ptr<std::thread> heartbeat_thread;
        bool simulation;
        
        //coms
        zmq::context_t context;
        zmq::socket_t send_socket;
        zmq::socket_t recv_socket;
        zmq::pollitem_t comm_items [1];

        //disable copy and assign
        drone(const drone&);
        drone & operator=(const drone&);

        //misc
        void load_config_vars();
        bool send_heartbeat(int lng, int lat, int alt, int bat_percentage);

        

        

};
