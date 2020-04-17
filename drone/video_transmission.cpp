#include "video_transmission.h"

#include <algorithm>  
#include "ascend_zmq.h"
#include "constants.h"
#include <iostream>
#include <zmq.hpp>
#include <spawn.h>
#include <csignal>
#include <unistd.h>

//for timing
#include <chrono>
#include <ctime>
#include <thread>

video_transmission::video_transmission(const std::string worker_address){
    this->worker_address=worker_address;
    start_transmission();
}

video_transmission::~video_transmission(){
    stop_transmission();
}

void video_transmission::start_transmission(){

    //generate SDP file

    //send SDP file to landing assist worker address

    //wait for go ahead to start streaming

    //start streaming

    //form command
    std::string full_cmd = '';

    //convert to proper command
    char cmd[full_cmd.size()+1];
    memcpy(cmd,full_cmd.c_str(),full_cmd.size()+1);
    cmd[full_cmd.size()];
    char *argv[] = {"sh", "-c", cmd, NULL};

    //execute
    int status;
    extern char** environ;
    status = posix_spawn(&pid, "/bin/sh", NULL,NULL,argv,environ);
}

void video_transmission::stop_transmission(){
    int status = kill(pid+1,SIGTERM);
}
