#include <iostream>
#include <string>
#include "drone.h"
#include "drone_msg.h"
#include "constants.h"
#include <chrono>
#include "config_handler.h"
#include "video_transmission.h"

//temp
#include <chrono>
#include <thread>
#include <unistd.h>

int main(){

    // //video transmission
    // video_transmission vid("random");
    // vid.start_transmission();
    // sleep(5);
    // vid.stop_transmission();
    // exit(0);


    drone ascendDrone;
    ascendDrone.manual();
    exit(0);

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

    //manual mode
    std::cout << "Entering Manual Mode..." << std::endl;
    ascendDrone.manual();
    std::cout << "...Exited Manual Mode" << std::endl;

    //land
    std::cout << "Landing initiated..." << std::endl;
    bool landing = ascendDrone.land();
    if(landing){std::cout << "...Landing" << std::endl;}
    else{std::cerr << "LANDING FAILURE" << std::endl;exit(1);}
    
    
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
