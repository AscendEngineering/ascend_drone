#pragma once

#include <memory>
#include <mavsdk/plugins/telemetry/telemetry.h>

using position = mavsdk::Telemetry::Position;


class sensors{

    public:
        
        sensors(std::shared_ptr<mavsdk::Telemetry> in_telemetry);
        ~sensors();

        //get functions
        position get_position();
        int get_num_satellites();
        float gps_strength();

    private:
        //disable copy and assign
        std::shared_ptr<mavsdk::Telemetry> telemetry;
        const int max_fixtype = 6;
        



};



