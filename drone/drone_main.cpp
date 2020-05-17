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


int main(int argc, char** argv){

    //Args
/*---------------------------------------------------------------*/
    cxxopts::Options options("Drone", "Runs on onboard computer to control flight controller");
    options.add_options()
        ("w,waypoint","Set flag to fly waypoints",cxxopts::value<bool>()->default_value("false"))
        ("t,test","test spin motors",cxxopts::value<bool>()->default_value("false"))
        ("s,simulation","connect to computer simulation",cxxopts::value<bool>()->default_value("false"))
        ("h,help", "Print usage");
    auto result = options.parse(argc, argv);

    if(result.count("help")){
        std::cout << options.help() << std::endl;
        exit(0);
    }

    bool waypoint_mode = result["waypoint"].as<bool>();
    bool test_motors = result["test"].as<bool>();

/*---------------------------------------------------------------*/


    drone ascendDrone(result["simulation"].as<bool>());

    //test motors
    if(test_motors){
        ascendDrone.test_motor();
        return 0;
    }

    //video open
    //send request to start streaming video

    //receive back the address of where to stream to

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

    //select the mode
    if(waypoint_mode){

        //waypoint mode
        std::cout << "Entering Waypoint Mode..." << std::endl;
        
        //create mission
        waypoints new_mission;
        new_mission.add_waypoint(-87.63976080858527,41.90018908454226,9.999999999999998);
        new_mission.add_waypoint(-87.63822525307192,41.89918538029315,9.999999999999998);
        
        //start mission
        bool succ_start = ascendDrone.start_mission(new_mission);

        //wait for completion
        if(succ_start){
            ascendDrone.wait_for_mission_completion();
        }
        std::cout << "...Exited Waypoint Mode" << std::endl;

    }
    else{
        //manual mode
        std::cout << "Entering Manual Mode..." << std::endl;
        ascendDrone.manual();
        std::cout << "...Exited Manual Mode" << std::endl;
    }

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
    
    
    
    // while(true){
    //     //receive messages
    //     std::vector<std::string> messages = ascendDrone.collect_messages();
    //     if(messages.size() > 0){
    //         std::cout<<"Messages"<<std::endl;
    //         for(auto msg: messages){
    //             std::cout<<"\t"<<msg<<std::endl;
    //             ascend::msg recvd_msg = msg_generator::deserialize(msg);
    //             if(recvd_msg.has_issue_landing()){
    //                 //start sending video off to specified landing worker
    //                 video_transmission vid("random");

    //                 //start accepting movement commands and relay those directly to n3
    //                 ascendDrone.collect_messages();
    //                 //for each message
    //                     //if terminate
    //                         //return
    //                     //else
    //                         //move drone according to message

    //                 //dont process any messages that might've come in during landing
    //                 ascendDrone.collect_messages();
    //             }





    //         }
    //     }

    //     //send message to ATC that we are ready to land
    //     std::string msg = msg_generator::generate_land_request(config_handler::instance()["drone_name"]);
    //     ascendDrone.send_to_atc(msg);

    // }

    //     //if time, send heartbeat
        



    //     // //heartbeat
    //     // auto now = std::chrono::system_clock::now();
    //     // std::chrono::duration<double> elapsed_time = now-last_heartbeat;
    //     // std::cout<<elapsed_time.count()<<std::endl;
    //     // if(elapsed_time.count() > 5.0){
    //     //     last_heartbeat=now;
    //     //     //send heartbeat
    //     //     std::cout<<"sending beat"<<std::endl;
    //     //     comm::connect(atc_socket,"tcp://localhost:" + constants::from_drone, "drone1");
    //     //     comm::send(atc_socket,msg_generator::generate_heartbeat(1,2,3,4));
    //     // }


    // }
    
    return 0;



}
