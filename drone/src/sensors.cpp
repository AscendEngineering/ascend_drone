#include "sensors.h"
#include <iostream>


sensors::sensors(std::shared_ptr<mavsdk::Telemetry> in_telemetry){

    telemetry = in_telemetry;

    //subscribe to each metric here

}

sensors::~sensors(){

}

position sensors::get_position(){
    return telemetry->position();
}

int sensors::get_num_satellites(){
    return telemetry->gps_info().num_satellites;
}

float sensors::gps_strength(){
    int fix_type = telemetry->gps_info().fix_type;
    return (float) fix_type/max_fixtype;
}