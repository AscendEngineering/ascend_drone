
#include "waypoints.h"
#include <mavsdk/plugins/mission/mission_item.h>


void waypoints::add_waypoints(::ascend::waypointList_msg waypoint_list){
}

void waypoints::add_waypoints(const std::vector<std::shared_ptr<mavsdk::MissionItem> >& waypoints){
    m_waypoints.insert(m_waypoints.end(),waypoints.begin(),waypoints.end());
}

void waypoints::add_waypoint(double latitude, double longitude, double altitude, double speed /*=1*/){
    std::shared_ptr<mavsdk::MissionItem> temp_item = std::make_shared<mavsdk::MissionItem>();
    temp_item->set_speed(speed);
    temp_item->set_position(latitude,longitude);
    temp_item->set_relative_altitude(altitude);
    m_waypoints.push_back(temp_item);
}

const std::vector<std::shared_ptr<mavsdk::MissionItem>>& waypoints::get_waypoints() const{
    return m_waypoints;
}

void waypoints::print_mission(){
    
    std::cout << "\n_______MISSION________" <<std::endl;

    for(int i =0; i< m_waypoints.size(); i++){
        std::cout << i << "."
                << "\tLat: " << m_waypoints[i]->get_latitude_deg() << "\n"
                << "\tLong: " << m_waypoints[i]->get_longitude_deg() << "\n"
                << "\tAlt: " << m_waypoints[i]->get_relative_altitude_m() << "\n"
                << "\tSpeed: " << m_waypoints[i]->get_speed_m_s() << "\n";
        std::cout << std::endl;
    }

    std::cout << "______________________" <<std::endl;
    
}

void waypoints::output_kml(){
    std::cout << "todo" << std::endl;
}





