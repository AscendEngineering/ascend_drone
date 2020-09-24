#include "drone.h"
#include <chrono>


int main(int argc, char** argv){
    
    drone ascendDrone(false);
    while(!ascendDrone.kill()){
        std::this_thread::sleep_for(std::chrono::seconds(1));
    };

}
