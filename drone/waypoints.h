#pragma once

#include <mavsdk/plugins/mission/mission.h>
#include "msgDef.pb.h"
#include <mavsdk/mavsdk.h>
#include <vector> 
#include <memory>

using namespace mavsdk;


class waypoints{

    public:
        //constructor
        waypoints(System& system);

        //upload waypoints
        void add_waypoints(const std::vector<std::shared_ptr<mavsdk::MissionItem> >& waypoints);
        void add_waypoints(::ascend::waypointList_msg waypoint_list);
        void add_waypoint(double latitude, double longitude, double altitude, double speed);
        bool upload_waypoints();

        //mission info
        bool mission_finished();
        int current_mission_item();
        int total_mission_items();

        //mission control methods
        bool start_mission();
        bool pause_mission();
        bool cancel_mission();

        void wait_for_completion();

    private:

        enum control_cmd{
            START,
            PAUSE,
            CANCEL
        };

        //variables
        std::shared_ptr<Mission> m_mission;
        std::vector<std::shared_ptr<mavsdk::MissionItem>> m_waypoints;

        //methods
        bool mission_control(control_cmd cmd);
};


