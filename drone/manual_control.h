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

};






