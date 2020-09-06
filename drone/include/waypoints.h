#pragma once

#include <mavsdk/plugins/mission/mission.h>
#include "msgDef.pb.h"
#include <mavsdk/mavsdk.h>
#include <vector> 
#include <memory>

using namespace mavsdk;


class waypoints{

    public:

        //upload waypoints
        void add_waypoints(const std::vector<std::shared_ptr<mavsdk::Mission::MissionItem> >& waypoints);
        //void add_waypoints(::ascend::waypointList_msg waypoint_list);
        void add_waypoint(double latitude, double longitude, double altitude, double speed=1);

        const std::vector<std::shared_ptr<mavsdk::Mission::MissionItem>>& get_waypoints() const;
        
        //print functions
        void print_mission();
        void output_kml();

    private:

        std::vector<std::shared_ptr<mavsdk::Mission::MissionItem>> m_waypoints;
        
};


