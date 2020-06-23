#pragma once

#include <string>

class video_transmission{
    

//this class should transmit video until destroyed or
//called stop on

public:
    video_transmission(const std::string& worker_address);
    ~video_transmission();

private:

    void start_transmission();
    void stop_transmission();

    video_transmission& operator=(const video_transmission&);
    video_transmission(const video_transmission&);
    std::string worker_address;
    pid_t pid;
};



