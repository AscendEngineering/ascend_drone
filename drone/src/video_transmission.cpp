#include "video_transmission.h"

#include <algorithm>  
#include "ascend_zmq.h"
#include "constants.h"
#include <iostream>
#include <zmq.hpp>
#include <spawn.h>
#include <csignal>
#include <unistd.h>
#include <fstream> 

//for timing
#include <chrono>
#include <ctime>


video_transmission::video_transmission(const std::string& worker_address){
    this->worker_address=worker_address;
    start_transmission();
}

video_transmission::~video_transmission(){
    stop_transmission();
}

void video_transmission::start_transmission(){
    
    //delete file in /var/tmp/video_ip 
    int result = remove(constants::video_file.c_str());

    //create file in /var/tmp/video_ip and write the ip to it
    std::ofstream outfile (constants::video_file);
    outfile << worker_address << std::endl;
    outfile.close();

    //make sure video closes when exiting all cases
    auto stop_trans = 
        [] (int i) { system("systemctl --user stop video_transmission"); std::cout << "stopped" << std::endl;exit(0); };

    //^C
    signal(SIGINT, stop_trans);
    //abort()
    signal(SIGABRT, stop_trans);
    //sent by "kill" command
    signal(SIGTERM, stop_trans);
    signal(SIGKILL, stop_trans);
    //^Z
    signal(SIGTSTP, stop_trans);
    
    //start the video transmission service
    system("systemctl --user start video_transmission");
}

void video_transmission::stop_transmission(){
    std::cout << "Stopping transmission..." << std::endl;
    system("systemctl --user stop video_transmission");
}
