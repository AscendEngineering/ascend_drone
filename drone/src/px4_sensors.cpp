#include "px4_sensors.h"
#include <iostream>


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
    std::cout << "latitude_deg: " << temp_pos.latitude_deg << std::endl;
    std::cout << "longitude_deg: " << temp_pos.longitude_deg << std::endl;
    std::cout << "absolute_altitude_m: " << temp_pos.absolute_altitude_m << std::endl;
    std::cout << "relative_altitude_m: " << temp_pos.relative_altitude_m << std::endl;
    std::cout << std::endl;

    //velocity
    velocity temp_velocity = get_velocity();
    std::cout << "north_m_s: " << temp_velocity.north_m_s << std::endl;
    std::cout << "east_m_s: " << temp_velocity.east_m_s << std::endl;
    std::cout << "down_m_s: " << temp_velocity.down_m_s << std::endl;

    //other
    std::cout << "get_num_satellites: " << get_num_satellites() << std::endl;
    std::cout << "gps_strength: " << gps_strength() << std::endl;
    std::cout << "get_battery: " << get_battery() << std::endl;
    std::cout << "get_flightmode: " << get_flightmode() << std::endl;
    std::cout << "is_gyro_calibrated: " << is_gyro_calibrated() << std::endl;
    std::cout << "is_accelerometer_calibrated: " << is_accelerometer_calibrated() << std::endl;
    std::cout << "is_magmeter_calibrated: " << is_magmeter_calibrated() << std::endl;
    std::cout << "is_level_calibrated: " << is_level_calibrated() << std::endl;
    std::cout << "is_local_position_ok: " << is_local_position_ok() << std::endl;
    std::cout << "is_global_position_ok: " << is_global_position_ok() << std::endl;
    std::cout << "is_home_position_ok: " << is_home_position_ok() << std::endl;
    std::cout << "get_roll: " << get_roll() << std::endl;
    std::cout << "get_pitch: " << get_pitch() << std::endl;
    std::cout << "get_yaw: " << get_yaw() << std::endl;

}

