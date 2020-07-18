#include "package_control.h"
#include <wiringPi.h>


void package_control::flip_switch(){
    digitalWrite(0, !magnet_on);
    magnet_on = !magnet_on;
}

void package_control::pickup(){
    if(!magnet_on){
        digitalWrite(0, true);
        magnet_on = true;
    }
}

void package_control::release(){
    if(magnet_on){
        digitalWrite(0, false);
        magnet_on = false;
    }
}

package_control::package_control(){
    bool magnet_on = false;
    wiringPiSetup();			// Setup the library
    pinMode(package_pin, OUTPUT);	
}

