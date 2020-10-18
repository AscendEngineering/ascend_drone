#pragma once

#include <mavsdk/mavsdk.h>
#include "package_control.h"
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include "external_sensors.h"

// NOTES

/*
    offboard->set_velocity_body({0.0f, 0.0f, 0.0f, 0.0f});
    offboard->set_velocity_body({forward, right, down, yaw});
*/


using namespace mavsdk;

class manual_control{

    public:
        manual_control(System* system);//,std::shared_ptr<mavsdk::Action> action);

    private:
        void translateKeyPress(char key, float& forward, float& right, float& down, float& yaw_right, float& rate);

        const float HORIZONTAL_INCREMENTS = 1;
        const float VERTICAL_INCREMENTS = 0.5;
        const float YAW_INCREMENTS = 18; //degrees per second (works out to 90 degress in 5 seconds)

        external_sensors sensor_group;
    
};






