#pragma once

#include <memory>
#include <mavsdk/plugins/telemetry/telemetry.h>

using position = mavsdk::Telemetry::Position;
using velocity = mavsdk::Telemetry::VelocityNED;


class sensors{

    public:
        
        sensors(std::shared_ptr<mavsdk::Telemetry> in_telemetry);

        //get functions
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

    private:
        //disable copy and assign
        std::shared_ptr<mavsdk::Telemetry> telemetry;
        const int max_fixtype = 6;
        



};



