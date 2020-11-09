
#include "waypoints.h"
#include <mavsdk/plugins/mission/mission.h>
#include "loguru.hpp"


// void waypoints::add_waypoints(::ascend::waypointList_msg waypoint_list){
// }

void waypoints::add_waypoints(const std::vector<std::shared_ptr<mavsdk::Mission::MissionItem> >& waypoints){
    m_waypoints.insert(m_waypoints.end(),waypoints.begin(),waypoints.end());
}

void waypoints::add_waypoint(double latitude, double longitude, double altitude, double speed /*=1*/){
    std::shared_ptr<mavsdk::Mission::MissionItem> temp_item = std::make_shared<mavsdk::Mission::MissionItem>();
    temp_item->speed_m_s = speed;
    temp_item->longitude_deg = longitude;
    temp_item->latitude_deg = longitude;
    temp_item->relative_altitude_m = altitude;
    m_waypoints.push_back(temp_item);
}

const std::vector<std::shared_ptr<mavsdk::Mission::MissionItem>>& waypoints::get_waypoints() const{
    return m_waypoints;
}

void waypoints::print_mission(){
    
    LOG_S(INFO) << "\n_______MISSION________" <<std::endl;

    for(int i =0; i< m_waypoints.size(); i++){
        LOG_S(INFO) << i << "."
                << "\tLat: " << m_waypoints[i]->latitude_deg << "\n"
                << "\tLong: " << m_waypoints[i]->longitude_deg << "\n"
                << "\tAlt: " << m_waypoints[i]->relative_altitude_m << "\n"
                << "\tSpeed: " << m_waypoints[i]->speed_m_s << "\n";
        LOG_S(INFO) << std::endl;
    }

    LOG_S(INFO) << "______________________" <<std::endl;
    
}

void waypoints::output_kml(){
    std::cerr << "todo" << std::endl;
}





