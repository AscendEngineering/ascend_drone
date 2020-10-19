#pragma once

#define ULTRA_ENABLED 0

class external_sensors {

    public:
        external_sensors();
        double get_ultrasonic_distance();

    private:
        const int ECHO_PIN = 5;
        const int TRIG_PIN = 1;
        const double SONIC_SPEED = 0.0000343; // cm per nanosecond

};


