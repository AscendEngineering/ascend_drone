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
#include <mavsdk/plugins/shell/shell.h>
#include <mavsdk/plugins/offboard/offboard.h>
#include "waypoints.h"
#include <memory>
#include "px4_sensors.h"
#include <thread>
#include "external_sensors.h"
#include <opencv2/opencv.hpp>
#include <time.h>

extern "C" {
#include <apriltag/apriltag.h>
#include <apriltag/apriltag_pose.h>
#include <apriltag/tag36h11.h>
#include <apriltag/tag25h9.h>
#include <apriltag/tag16h5.h>
#include <apriltag/tagCircle21h7.h>
#include <apriltag/tagCircle49h12.h>
#include <apriltag/tagCustom48h12.h>
#include <apriltag/tagStandard41h12.h>
#include <apriltag/tagStandard52h13.h>
#include <apriltag/common/getopt.h>
}

static const double YAW_FACTOR = 180.00;
using namespace cv;

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
        void control_from_remote(bool april_assist=false);
        void test_motor(int motor = -1);
        void calibrate(int sensor = -1);
        void get_px4log();

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
        bool april_land(std::shared_ptr<Offboard> offboard);

        //waypoints
        enum control_cmd{
            START,
            PAUSE,
            CANCEL
        };
        bool mission_control(control_cmd cmd);
        bool upload_waypoints(const mavsdk::Mission::MissionPlan& mission_plan);
        std::shared_ptr<Mission> m_mission;
        
        //vars
        std::string drone_name;
        std::string atc_ip;
        mavsdk::Mavsdk px4;
        mavsdk::System* system;
        std::shared_ptr<mavsdk::Telemetry> telemetry;
        std::shared_ptr<mavsdk::Action> action;
        std::shared_ptr<px4_sensors> drone_sensors;
        std::shared_ptr<std::thread> heartbeat_thread;
        bool simulation;
        external_sensors sensor_group;

        //april tag definitions
        int april_debug = 0;
        int april_quiet = 0;
        std::string april_tag_family = "tag36h11";
        int april_threads = 1;
        float april_decimate = 2.0;
        float april_blur = 0.0;
        int april_refine_edges = 1;
        
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
        bool send_heartbeat(double lng, double lat, double alt, double bat_percentage);

        

        

};
