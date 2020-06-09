#include <iostream>
#include <string>
#include "drone.h"
#include "drone_msg.h"
#include "constants.h"
#include <chrono>
#include "config_handler.h"
#include "video_transmission.h"
#include <cxxopts.hpp>
#include <chrono>
#include <thread>
#include <unistd.h>
#include "waypoints.h"


namespace {

    void process_request(const ascend::msg& recvd_msg, drone& ascendDrone){

        //process request
        if(recvd_msg.has_takeoff_request()){

            //arm
            std::cout << "Arming..." << std::endl;
            bool armed = ascendDrone.arm();
            if(armed){std::cout << "...Armed" << std::endl;}
            else{std::cerr << "ARMING FAILURE" << std::endl;exit(1);}
            
            //takeoff
            std::cout << "Taking off..." << std::endl;
            bool tookoff = ascendDrone.takeoff();
            if(tookoff){std::cout << "...Took off" << std::endl;}
            else{std::cerr << "TAFEOFF FAILURE" << std::endl;exit(1);}

            std::this_thread::sleep_for (std::chrono::seconds(10));
        }
        else if(recvd_msg.has_landing_request()){
            
            //land
            std::cout << "Landing initiated..." << std::endl;
            bool landing = false;
            while(!landing){
                landing = ascendDrone.land();
                if(landing){
                    std::cout << "...Landing" << std::endl;
                }
                else{
                    std::cerr << "LANDING FAILURE...RETRY" << std::endl;
                    std::this_thread::sleep_for (std::chrono::seconds(5));
                }
            }
        }
        else if(recvd_msg.has_waypoints()){
            
            //waypoints
            std::cout << "Entering Waypoint Mode..." << std::endl;
            waypoints new_mission;
            new_mission.add_waypoint(-87.63976080858527,41.90018908454226,9.999999999999998,5);
            new_mission.add_waypoint(-87.639227,41.899937,9.999999999999998,5);
            
            //start mission
            bool succ_start = ascendDrone.start_mission(new_mission);
            if(succ_start){
                ascendDrone.wait_for_mission_completion();
            }
            std::cout << "...Exited Waypoint Mode" << std::endl;
        }
        else if(recvd_msg.has_manual_control()){

            //manual control
            std::cout << "Entering Manual Mode..." << std::endl;
            ascendDrone.manual();
            std::cout << "...Exiting Manual Mode" << std::endl;
        }
    }    
}




int main(int argc, char** argv){

    //Args
/*---------------------------------------------------------------*/
    cxxopts::Options options("Drone", "Runs on onboard computer to control flight controller");
    options.add_options()
        ("t,test","test spin motors",cxxopts::value<bool>()->default_value("false"))
        ("s,simulation","connect to computer simulation",cxxopts::value<bool>()->default_value("false"))
        ("h,help", "Print usage");
    auto cmd_line_args = options.parse(argc, argv);

    if(cmd_line_args.count("help")){
        std::cout << options.help() << std::endl;
        exit(0);
    }

    bool test_motors = cmd_line_args["test"].as<bool>();
    bool run_simulation = cmd_line_args["simulation"].as<bool>();

/*---------------------------------------------------------------*/

    //variables
    std::chrono::time_point<std::chrono::system_clock> last_heartbeat = std::chrono::system_clock::now();
    drone ascendDrone(run_simulation);

    //test motors
    if(test_motors){
        std::cout << "Testing Motor" << std::endl;
        ascendDrone.test_motor();
        return 0;
    }
    
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
    
    return 0;
}
