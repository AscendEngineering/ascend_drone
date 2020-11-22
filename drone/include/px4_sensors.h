#pragma once

#include <memory>
#include <mavsdk/plugins/telemetry/telemetry.h>

using position = mavsdk::Telemetry::Position;
    //double latitude_deg
    //double longitude_deg
    //float absolute_altitude_m
    //float relative_altitude_m 

using velocity = mavsdk::Telemetry::VelocityNed;
    //float north_m_s
    //float east_m_s
    //float down_m_s

using distance = mavsdk::Telemetry::DistanceSensor;
    //minimum_distance_m
    //maximum_distance_m
    //current_distance_m

#define TFMINI_ENABLED 0

class px4_sensors{

    public:
        
        px4_sensors(std::shared_ptr<mavsdk::Telemetry> in_telemetry);

        //get functions
        distance get_distance();
        position get_position();
        velocity get_velocity();
        int get_num_satellites();
        float gps_strength();
        float get_battery();
        std::string get_flightmode();
        bool is_gyro_calibrated();
        bool is_accelerometer_calibrated();
        bool is_magmeter_calibrated();
        bool is_level_calibrated();
        bool is_local_position_ok();
        bool is_global_position_ok();
        bool is_home_position_ok();
        float get_roll();
        float get_pitch();
        float get_yaw();

        void print_all();

    private:
        //disable copy and assign
        std::shared_ptr<mavsdk::Telemetry> telemetry;
        const int max_fixtype = 6;
        



};



