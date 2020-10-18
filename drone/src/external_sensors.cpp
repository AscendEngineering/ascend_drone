#include "external_sensors.h"
#include <wiringPi.h>
#include <chrono>


external_sensors::external_sensors(){
    wiringPiSetup();
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
}

double external_sensors::get_ultrasonic_distance(){

    //output sonar
    digitalWrite(TRIG_PIN,HIGH);
    delay(0.01);
    digitalWrite(TRIG_PIN,LOW);

    //
    auto start = std::chrono::system_clock::now();
    auto end = std::chrono::system_clock::now();

    while(digitalRead(ECHO_PIN)==0){
        start = std::chrono::system_clock::now();
    }
    while(digitalRead(ECHO_PIN)==1){
        end = std::chrono::system_clock::now();
    }

    std::chrono::duration<double, std::nano> time_elapsed(end-start);
    double distance = (time_elapsed.count() * SONIC_SPEED)/2.0;

    return distance;
}


