
#include "waypoints.h"
#include <mavsdk/plugins/mission/mission_item.h>
#include <future>


waypoints::waypoints(System& system){
    m_mission = std::make_shared<Mission>(system);
}

void waypoints::add_waypoints(::ascend::waypointList_msg waypoint_list){
}

void waypoints::add_waypoints(const std::vector<std::shared_ptr<mavsdk::MissionItem> >& waypoints){
    m_waypoints.insert(m_waypoints.end(),waypoints.begin(),waypoints.end());
}

void waypoints::add_waypoint(float latitude, float longitude, float altitude, float speed){
    std::shared_ptr<mavsdk::MissionItem> temp_item = std::make_shared<mavsdk::MissionItem>();
    temp_item->set_speed(speed);
    temp_item->set_position(latitude,longitude);
    temp_item->set_relative_altitude(altitude);
    m_waypoints.push_back(temp_item);
}

bool waypoints::upload_waypoints(){

    //setup promise/future
    auto prom = std::make_shared<std::promise<Mission::Result>>();
    auto future_result = prom->get_future();

    //for each waypoint
    m_mission->upload_mission_async(m_waypoints, [prom](Mission::Result result) { prom->set_value(result); });
    const Mission::Result result = future_result.get();
    
    return (result==Mission::Result::SUCCESS);
}


bool waypoints::mission_finished(){
    return m_mission->mission_finished();
}

int waypoints::current_mission_item(){
    return m_mission->current_mission_item();
}

int waypoints::total_mission_items(){
    return m_mission->total_mission_items();
}

bool waypoints::start_mission(){
    return mission_control(START);
}

bool waypoints::pause_mission(){
    return mission_control(PAUSE);
}

bool waypoints::cancel_mission(){
    return mission_control(CANCEL);
}

void waypoints::wait_for_completion(){

    //setup variables
    bool finished = false;
    auto prom = std::make_shared<std::promise<float>>();
    auto future_result = prom->get_future();

    //subscribe to the updates 
    m_mission->subscribe_progress([prom](int current, int total) { prom->set_value((float)(current/total));});

    //wait until reached last waypoint
    while(!finished){
        float result = future_result.get();
        if(result == 1.0){
            finished = true;
        }
    }
}

bool waypoints::mission_control(control_cmd cmd){
    
    auto prom = std::make_shared<std::promise<Mission::Result>>();
    auto future_result = prom->get_future();

    switch(cmd){
        case START:{
            m_mission->start_mission_async([prom](Mission::Result result) { prom->set_value(result);});
            break;
        }
        case PAUSE:{
            m_mission->pause_mission_async([prom](Mission::Result result) {prom->set_value(result);});
            break;
        }
        case CANCEL:{
            m_mission->clear_mission_async([prom](Mission::Result result) {prom->set_value(result);});
            break;
        }
        default:{
            std::cerr << "Invalid command" << std::endl;
            return false;
        }

    }
    
    const Mission::Result result = future_result.get();
    return (result==Mission::Result::SUCCESS);
}





