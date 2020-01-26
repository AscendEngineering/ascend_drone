#pragma once

#include <string>

#include <raspicam/raspicam_cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

class video_transmission{
    

//this class should transmit video until destroyed or
//called stop on

public:
    video_transmission(const std::string worker_address);
    void start_transmission();
    void stop_transmission();


private:
    video_transmission& operator=(const video_transmission&);
    video_transmission(const video_transmission&);
    std::string worker_address;
    raspicam::RaspiCam_Cv Camera; 

};



