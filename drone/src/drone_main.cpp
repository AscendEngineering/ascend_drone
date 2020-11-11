#include "config_handler.h"
#include "constants.h"
#include "drone.h"
#include "drone_msg.h"
#include "loguru.hpp"
#include "utilities.h"
#include "waypoints.h"

#include <chrono>
#include <cxxopts.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>


#define MESSAGE_DEBUGGING 0

namespace {

    void process_request(const ascend::msg& recvd_msg, drone& ascendDrone){

        //TODO
    }    
}

void exiting() {
    std::cerr << "Exiting" << std::endl;
}


int main(int argc, char** argv){


    //logging
    std::string timestamp = utilities::timestamp();
    loguru::init(argc, argv);
    loguru::add_file( ("logs/"+timestamp+".log").c_str() , loguru::Append, loguru::Verbosity_INFO);

    //Args
/*---------------------------------------------------------------*/
    cxxopts::Options options("Drone", "Runs on onboard computer to control flight controller");
    options.add_options()
        ("t,test","test spin motors",cxxopts::value<bool>()->default_value("false"))
        ("s,simulation","connect to computer simulation",cxxopts::value<bool>()->default_value("false"))
        ("m,manual","manual flight mode",cxxopts::value<bool>()->default_value("false"))
        ("c,calibrate","enter drone calibration",cxxopts::value<bool>()->default_value("false"))
        ("h,help", "Print usage");
    auto cmd_line_args = options.parse(argc, argv);

    if(cmd_line_args.count("help")){
        std::cerr << options.help() << std::endl;
        exit(0);
    }

    bool calibrate = cmd_line_args["calibrate"].as<bool>();
    bool test_motors = cmd_line_args["test"].as<bool>();
    bool run_simulation = cmd_line_args["simulation"].as<bool>();
    bool manual_mode = cmd_line_args["manual"].as<bool>();

/*---------------------------------------------------------------*/

    //variables
    std::chrono::time_point<std::chrono::system_clock> last_heartbeat = std::chrono::system_clock::now();
    drone ascendDrone(run_simulation);

    //calibrate
    if(calibrate){

        int sensor = -1;

        std::cerr << "What would you like to calibrate?" << std::endl;
        std::cerr << "1) Gyro" << std::endl;
        std::cerr << "2) Level Horizon" << std::endl;
        std::cerr << "3) Accelerometer" << std::endl;
        std::cerr << "4) Magnetometer" << std::endl;
        std::cerr << "5) All" << std::endl;
        std::cin >> sensor;

        //validity
        if(sensor < -1 || sensor > 5){
            std::cerr << "INVALID ENTRY, TRY AGAIN" << std::endl;
            return 1;
        }
        if(sensor==5){
            sensor = -1;
        }

        //calibrate
        ascendDrone.calibrate(sensor);
        std::cerr << "Calibration Done" << std::endl;
        return 0;
    }

    //test motors
    if(test_motors){
        std::cerr << "Testing Motor" << std::endl;
        ascendDrone.test_motor();
        return 0;
    }

    if(manual_mode){
        ascendDrone.manual();
    }
    else{
        
        //wait for commands from ATC
        while(true){
            std::vector<std::string> messages = ascendDrone.collect_messages();

            if(messages.size() > 0){
                for(auto msg: messages){
                    ascend::msg recvd_msg = msg_generator::deserialize(msg);
                    ::process_request(recvd_msg, ascendDrone);
                }
            }

            //heartbeat
            auto now = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_time = now-last_heartbeat;
            if(elapsed_time.count() > 5.0){
                last_heartbeat=now;
                ascendDrone.send_heartbeat();
            } 
        }
    }
    
    ascendDrone.land();

    ascendDrone.save_px4log(timestamp);

    return 0;
}
