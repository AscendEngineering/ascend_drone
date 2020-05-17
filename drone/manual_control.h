#pragma once

#include <mavsdk/mavsdk.h>

// NOTES

/*
    offboard->set_velocity_body({0.0f, 0.0f, 0.0f, 0.0f});
    offboard->set_velocity_body({forward, right, down, yaw});
*/


using namespace mavsdk;

class manual_control{

    public:
        manual_control(System* system);

    private:
        void translateKeyPress(char key, float& forward, float& right, float& down, float& yaw_right, float& rate);
        const float HORIZONTAL_INCREMENTS = 1.0;
        const float VERTICAL_INCREMENTS = 0.2;
        const float YAW_INCREMENTS = 18;

};






