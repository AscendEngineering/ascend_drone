#pragma once

#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/offboard/offboard.h>
#include <opencv2/opencv.hpp>
#include <time.h>
#include "loguru.hpp"

#include "px4_sensors.h"

extern "C" {
    #include <apriltag/apriltag.h>
    #include <apriltag/apriltag_pose.h>
    #include <apriltag/tag36h11.h>
    #include <apriltag/tag25h9.h>
    #include <apriltag/tag16h5.h>
    #include <apriltag/tagCircle21h7.h>
    #include <apriltag/tagCircle49h12.h>
    #include <apriltag/tagCustom48h12.h>
    #include <apriltag/tagStandard41h12.h>
    #include <apriltag/tagStandard52h13.h>
    #include <apriltag/common/getopt.h>
}

using namespace mavsdk;
using namespace cv;

class april_land{

    public:
        april_land();
        bool execute(std::shared_ptr<Offboard> offboard,std::shared_ptr<px4_sensors> drone_sensors, bool simulation = false);

    private:

        //april tag definitions
        int april_debug = 0;
        int april_quiet = 0;
        std::string april_tag_family = "tag36h11";
        int april_threads = 1;
        float april_decimate = 2.0;
        float april_blur = 0.0;
        int april_refine_edges = 1;

        //vars
        apriltag_detector_t *td = nullptr;

        //functions
        float rate_calculator(float altitude);
        int april_mover_y(int frame_y, double april_y, int distance);
        int april_mover_x(int frame_x, double april_x, int distance);
        int april_mover_yaw(double yaw_percentage, int x, int y);
        int april_mover_z(int x, int y, float altitude);
        double april_timer(float altitude, int x, int y, int yaw_change);
};










