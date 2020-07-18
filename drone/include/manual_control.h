#pragma once

#include <mavsdk/mavsdk.h>
#include "package_control.h"
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/telemetry/telemetry.h>

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
        // void manual_takeoff(std::shared_ptr<mavsdk::Action> action);
        // void manual_land(std::shared_ptr<mavsdk::Action> action);

        const float HORIZONTAL_INCREMENTS = 1.0;
        const float VERTICAL_INCREMENTS = 0.2;
        const float YAW_INCREMENTS = 18; //degrees per second (works out to 90 degress in 5 seconds)

};






