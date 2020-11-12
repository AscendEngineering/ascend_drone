#include "px4_sensors.h"
#include <iostream>
#include "loguru.hpp"


px4_sensors::px4_sensors(std::shared_ptr<mavsdk::Telemetry> in_telemetry){

    telemetry = in_telemetry;

    //subscribe to each metric here
}

distance px4_sensors::get_distance(){
    return telemetry->distance_sensor();
}
position px4_sensors::get_position(){
    return telemetry->position();
}
velocity px4_sensors::get_velocity(){
    return telemetry->position_velocity_ned().velocity;
}
int px4_sensors::get_num_satellites(){
    return telemetry->gps_info().num_satellites;
}
float px4_sensors::gps_strength(){
    int fix_type = (int)telemetry->gps_info().fix_type;
    return (float) fix_type/max_fixtype;
}
float px4_sensors::get_battery(){
    float battery = telemetry->battery().remaining_percent;
    battery *= 100;
    return battery;
}
std::string px4_sensors::get_flightmode(){
    std::string retval;

    using fm = mavsdk::Telemetry::FlightMode;
    fm mode = telemetry->flight_mode();

    switch(mode){
        case fm::Unknown:{
            retval = "UNKNOWN";
            break;
        }
        case fm::Ready:{
            retval = "READY";
            break;
        }
        case fm::Takeoff:{
            retval = "TAKEOFF";
            break;
        }
        case fm::Hold:{
            retval = "HOLD";
            break;
        }
        case fm::Mission:{
            retval = "MISSION";
            break;
        }
        case fm::ReturnToLaunch:{
            retval = "RETURN_TO_LAUNCH";
            break;
        }
        case fm::Land:{
            retval = "LAND";
            break;
        }
        case fm::Offboard:{
            retval = "OFFBOARD";
            break;
        }
        case fm::FollowMe:{
            retval = "FOLLOW_ME";
            break;
        }
        case fm::Manual:{
            retval = "MANUAL";
            break;
        }
        case fm::Altctl:{
            retval = "ALTCTL";
            break;
        }
        case fm::Posctl:{
            retval = "POSCTL";
            break;
        }
        case fm::Acro:{
            retval = "ACRO";
            break;
        }
        case fm::Stabilized:{
            retval = "STABILIZED";
            break;
        }
        case fm::Rattitude:{
            retval = "RATTITUDE";
            break;
        }
        default: {
            retval = "UNKNOWN";
            break;
        }
    }

    return retval;
}

bool px4_sensors::is_gyro_calibrated(){
    return telemetry->health().is_gyrometer_calibration_ok;
}
bool px4_sensors::is_accelerometer_calibrated(){
    return telemetry->health().is_accelerometer_calibration_ok;
}
bool px4_sensors::is_magmeter_calibrated(){
    return telemetry->health().is_magnetometer_calibration_ok;
}
bool px4_sensors::is_level_calibrated(){
    return telemetry->health().is_level_calibration_ok;
}
bool px4_sensors::is_local_position_ok(){
    return telemetry->health().is_local_position_ok;
}
bool px4_sensors::is_global_position_ok(){
    return telemetry->health().is_global_position_ok;
}
bool px4_sensors::is_home_position_ok(){
    return telemetry->health().is_home_position_ok;
}
float px4_sensors::get_roll(){
    return telemetry->attitude_euler().roll_deg;
}
float px4_sensors::get_pitch(){
    return telemetry->attitude_euler().pitch_deg;
}
float px4_sensors::get_yaw(){
    return telemetry->attitude_euler().yaw_deg;
}

void px4_sensors::print_all(){

    //position
    position temp_pos = get_position();
    LOG_S(INFO)<< "latitude_deg: " << temp_pos.latitude_deg;
    LOG_S(INFO)<< "longitude_deg: " << temp_pos.longitude_deg;
    LOG_S(INFO)<< "absolute_altitude_m: " << temp_pos.absolute_altitude_m;
    LOG_S(INFO)<< "relative_altitude_m: " << temp_pos.relative_altitude_m;
    LOG_S(INFO)<< std::endl;

    //velocity
    velocity temp_velocity = get_velocity();
    LOG_S(INFO)<< "north_m_s: " << temp_velocity.north_m_s;
    LOG_S(INFO)<< "east_m_s: " << temp_velocity.east_m_s;
    LOG_S(INFO)<< "down_m_s: " << temp_velocity.down_m_s;

    //other
    LOG_S(INFO)<< "get_num_satellites: " << get_num_satellites();
    LOG_S(INFO)<< "gps_strength: " << gps_strength();
    LOG_S(INFO)<< "get_battery: " << get_battery();
    LOG_S(INFO)<< "get_flightmode: " << get_flightmode();
    LOG_S(INFO)<< "is_gyro_calibrated: " << is_gyro_calibrated();
    LOG_S(INFO)<< "is_accelerometer_calibrated: " << is_accelerometer_calibrated();
    LOG_S(INFO)<< "is_magmeter_calibrated: " << is_magmeter_calibrated();
    LOG_S(INFO)<< "is_level_calibrated: " << is_level_calibrated();
    LOG_S(INFO)<< "is_local_position_ok: " << is_local_position_ok();
    LOG_S(INFO)<< "is_global_position_ok: " << is_global_position_ok();
    LOG_S(INFO)<< "is_home_position_ok: " << is_home_position_ok();
    LOG_S(INFO)<< "get_roll: " << get_roll();
    LOG_S(INFO)<< "get_pitch: " << get_pitch();
    LOG_S(INFO)<< "get_yaw: " << get_yaw();

}

