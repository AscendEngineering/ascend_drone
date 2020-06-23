#include "sensors.h"
#include <iostream>


sensors::sensors(std::shared_ptr<mavsdk::Telemetry> in_telemetry){

    telemetry = in_telemetry;

    //subscribe to each metric here
}

position sensors::get_position(){
    return telemetry->position();
}
velocity sensors::get_velocity(){
    return telemetry->position_velocity_ned().velocity;
}
int sensors::get_num_satellites(){
    return telemetry->gps_info().num_satellites;
}
float sensors::gps_strength(){
    int fix_type = telemetry->gps_info().fix_type;
    return (float) fix_type/max_fixtype;
}
float sensors::get_battery(){
    float battery = telemetry->battery().remaining_percent;
    battery *= 100;
    return battery;
}
std::string sensors::get_flightmode(){
    std::string retval;

    using fm = mavsdk::Telemetry::FlightMode;
    fm mode = telemetry->flight_mode();

    switch(mode){
        case fm::UNKNOWN:{
            retval = "UNKNOWN";
            break;
        }
        case fm::READY:{
            retval = "READY";
            break;
        }
        case fm::TAKEOFF:{
            retval = "TAKEOFF";
            break;
        }
        case fm::HOLD:{
            retval = "HOLD";
            break;
        }
        case fm::MISSION:{
            retval = "MISSION";
            break;
        }
        case fm::RETURN_TO_LAUNCH:{
            retval = "RETURN_TO_LAUNCH";
            break;
        }
        case fm::LAND:{
            retval = "LAND";
            break;
        }
        case fm::OFFBOARD:{
            retval = "OFFBOARD";
            break;
        }
        case fm::FOLLOW_ME:{
            retval = "FOLLOW_ME";
            break;
        }
        case fm::MANUAL:{
            retval = "MANUAL";
            break;
        }
        case fm::ALTCTL:{
            retval = "ALTCTL";
            break;
        }
        case fm::POSCTL:{
            retval = "POSCTL";
            break;
        }
        case fm::ACRO:{
            retval = "ACRO";
            break;
        }
        case fm::STABILIZED:{
            retval = "STABILIZED";
            break;
        }
        case fm::RATTITUDE:{
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

bool sensors::is_gyro_calibrated(){
    return telemetry->health().gyrometer_calibration_ok;
}
bool sensors::is_accelerometer_calibrated(){
    return telemetry->health().accelerometer_calibration_ok;
}
bool sensors::is_magmeter_calibrated(){
    return telemetry->health().magnetometer_calibration_ok;
}
bool sensors::is_level_calibrated(){
    return telemetry->health().level_calibration_ok;
}
bool sensors::is_local_position_ok(){
    return telemetry->health().local_position_ok;
}
bool sensors::is_global_position_ok(){
    return telemetry->health().global_position_ok;
}
bool sensors::is_home_position_ok(){
    return telemetry->health().home_position_ok;
}
float sensors::get_roll(){
    return telemetry->attitude_euler_angle().roll_deg;
}
float sensors::get_pitch(){
    return telemetry->attitude_euler_angle().pitch_deg;
}
float sensors::get_yaw(){
    return telemetry->attitude_euler_angle().yaw_deg;
}

void sensors::print_all(){

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

