#include "april_land.h"
#include <sstream>
#include <stdio.h>
#include <thread> 
#include <chrono>  
#include <cstdlib>
#include "utilities.h"


namespace {
    float adjust_yaw(float in_yaw, float rate){
        if(in_yaw==0 || rate ==0){
            return 0;
        }
        return in_yaw * (18+ (1/rate));
    }

    float get_height(std::shared_ptr<px4_sensors> drone_sensors){
        double sending_height = 5.0;
        distance tfmini_reading= drone_sensors->get_distance();
        bool in_range = !std::isnan(tfmini_reading.current_distance_m)
            && tfmini_reading.current_distance_m > tfmini_reading.minimum_distance_m
            && tfmini_reading.current_distance_m < tfmini_reading.maximum_distance_m;
        if(in_range){
            sending_height = (double)tfmini_reading.current_distance_m / (double)100;
        }
        else{
            position current_pos = drone_sensors->get_position();
            sending_height = current_pos.relative_altitude_m;
        }
        return sending_height;
    }
}

april_land::april_land(){
    
    //setup detector
    apriltag_family_t *tf = NULL;
    if (!strcmp(april_tag_family.c_str(), "tag36h11")) {
        tf = tag36h11_create();
    } else if (!strcmp(april_tag_family.c_str(), "tag25h9")) {
        tf = tag25h9_create();
    } else if (!strcmp(april_tag_family.c_str(), "tag16h5")) {
        tf = tag16h5_create();
    } else if (!strcmp(april_tag_family.c_str(), "tagCircle21h7")) {
        tf = tagCircle21h7_create();
    } else if (!strcmp(april_tag_family.c_str(), "tagCircle49h12")) {
        tf = tagCircle49h12_create();
    } else if (!strcmp(april_tag_family.c_str(), "tagStandard41h12")) {
        tf = tagStandard41h12_create();
    } else if (!strcmp(april_tag_family.c_str(), "tagStandard52h13")) {
        tf = tagStandard52h13_create();
    } else if (!strcmp(april_tag_family.c_str(), "tagCustom48h12")) {
        tf = tagCustom48h12_create();
    } else {
        tf = tag36h11_create(); //default        
    }
    
    td = apriltag_detector_create();
    apriltag_detector_add_family(td, tf);
    td->quad_decimate = april_decimate;
    td->quad_sigma = april_blur;
    td->nthreads = april_threads;
    td->debug = april_debug;
    td->refine_edges = april_refine_edges;
}

bool april_land::execute(std::shared_ptr<Offboard> offboard,std::shared_ptr<px4_sensors> drone_sensors, bool simulation){
    //offboard should already be initiated
    if(!offboard->is_active()){
        return false;
    }
    //initialize killswitch
    utilities::line_buffer(false);
    //kill any ffmpeg process to server
    FILE* kill_ffmpeg = popen("pkill -f ffmpeg","r");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    // Initialize camera
    cv::VideoCapture cap;
    if(simulation){
        setenv("DISPLAY", ":0", true);
        cap = cv::VideoCapture("udpsrc port=5600 ! application/x-rtp, payload=96 ! rtph264depay ! h264parse ! avdec_h264 ! decodebin ! videoconvert ! video/x-raw,format=(string)BGR ! videoconvert ! appsink emit-signals=true sync=false max-buffers=2 drop=true", cv::CAP_GSTREAMER);
    }
    else{
        cap = cv::VideoCapture(0);
    }
    //check camera
    if (!cap.isOpened()) {
        std::cerr << "Couldn't open video capture device" << std::endl;
        return -1;
    }
    float altitude = ::get_height(drone_sensors);
    double timer2 = 0;
    double timer_differential = 0;
    bool move_down = false;
    Mat frame, gray;
    while (true) {
        cap >> frame;
        int width = frame.cols;
        int height = frame.rows;
        int frame_x_center = width / 2;
        int frame_y_center = height / 2;
        cvtColor(frame, gray, COLOR_BGR2GRAY);
        // Make an image_u8_t header for the Mat data
        image_u8_t im = { .width = gray.cols,
            .height = gray.rows,
            .stride = gray.cols,
            .buf = gray.data
        };
        float x;
        float y;
        float z;
        float yaw;
        float rate;
        zarray_t *detections = apriltag_detector_detect(td, &im);
        // Draw detection outlines
        for (int i = 0; i < zarray_size(detections); i++) {
            apriltag_detection_t *det;
            zarray_get(detections, i, &det);
            line(frame, Point(det->p[0][0], det->p[0][1]),
                     Point(det->p[1][0], det->p[1][1]),
                     Scalar(0, 0xff, 0), 2);
            line(frame, Point(det->p[0][0], det->p[0][1]),
                     Point(det->p[3][0], det->p[3][1]),
                     Scalar(0, 0, 0xff), 2);
            line(frame, Point(det->p[1][0], det->p[1][1]),
                     Point(det->p[2][0], det->p[2][1]),
                     Scalar(0xff, 0, 0), 2);
            line(frame, Point(det->p[2][0], det->p[2][1]),
                     Point(det->p[3][0], det->p[3][1]),
                     Scalar(0xff, 0, 0), 2);
            altitude = ::get_height(drone_sensors);
            apriltag_detection_info_t info;
            if(simulation){
                info.tagsize = 1.0;
                info.fx = 277;
                info.fy = 277;
                info.cx = 160;
                info.cy = 120;
            }
            else{
                info.tagsize = .1;
                info.fx = 2191.82299;
                info.fy = 2145.07083;
                info.cx = 321.340863;
                info.cy = 174.398525;
            }
            info.det = det;
            apriltag_pose_t pose;
            double err = estimate_tag_pose(&info, &pose);
            double pi = 3.14159;
            double val = 180.0 / pi;
            //Eigen::Vector3d translation;
            //translation(0) = pose.t->data[0];
            //translation(1) = pose.t->data[1];
            //translation(2) = pose.t->data[2];
            double x1 = pose.R->data[0];
            double y1 = pose.R->data[1];
            double z1 = pose.R->data[2];
            double x2 = pose.R->data[3];
            double y2 = pose.R->data[4];
            double z2 = pose.R->data[5];
            double x3 = pose.R->data[6];
            double y3 = pose.R->data[7];
            double z3 = pose.R->data[8];
            double yaw_percentage = atan2(-y1, x1) * val;
            //april moving ros message defined here
            rate = rate_calculator(altitude);
            x = april_mover_x(frame_x_center, det->c[0], 50);
            y = april_mover_y(frame_y_center, det->c[1], 50);
            z = april_mover_z(x, y, altitude);
            yaw = april_mover_yaw(yaw_percentage, x, y);
            //april center calculation
            LOG_S(INFO)<< "X CENTER OF APRIL: " << det->c[0];
            LOG_S(INFO)<< "Y CENTER OF APRIL: " << det->c[1];
            LOG_S(INFO)<< "X integer: " << x;
            LOG_S(INFO)<< "Y integer: " << y;
            LOG_S(INFO)<< "Z: " << z;
            LOG_S(INFO)<< "Yaw integer: " << yaw;
            LOG_S(INFO)<< "Altitude: " << altitude;
            LOG_S(INFO)<< "Rate: " << rate;
            std::stringstream ss;
            ss << det->id;
            String text = ss.str();
            int fontface = FONT_HERSHEY_SCRIPT_SIMPLEX;
            double fontscale = 1.0;
            int baseline;
            Size textsize = getTextSize(text, fontface, fontscale, 2,
                                            &baseline);
            putText(frame, text, Point(det->c[0]-textsize.width/2,
                                       det->c[1]+textsize.height/2),
                    fontface, fontscale, Scalar(0xff, 0x99, 0), 2);
        }
        //call the movement here
        float x_move_rate = (float)x*rate;
        float y_move_rate = (float)y*rate;
        float z_move_rate = (float)z*rate;
        float yaw_move_rate = ::adjust_yaw(yaw, rate);
        mavsdk::Offboard::VelocityBodyYawspeed movement{};
        movement.right_m_s = x_move_rate;
        movement.forward_m_s = y_move_rate;
        movement.down_m_s = z_move_rate;
        movement.yawspeed_deg_s = yaw_move_rate;
        offboard->set_velocity_body(movement);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        double timer1 = april_timer(altitude, x, y, yaw);
        if(timer1 == 0){
            timer2 = 0;
        }
        if(timer2 != 0){
            double timer = timer1 - timer2;
            timer_differential += timer;
            timer_differential = abs(timer_differential);
        }
        timer2 = timer1;
        if(timer_differential >= 2.0){
        //   drone_land; <-- meant to be what calls land
            LOG_S(INFO) << "Landing";
            move_down = true;
            break;
        }
        //*************************************
        if(x != 0 && y != 0 && altitude > 2.0){
            timer_differential = 0;
        }
        LOG_S(INFO)<< "Timer Differential: " << timer_differential;
        LOG_S(INFO)<< "Timer1: " << timer1;
        LOG_S(INFO)<< "Timer2: " << timer2 << "\n";
        apriltag_detections_destroy(detections);
        imshow("Tag Detections", frame);
        char c = (char)waitKey(1);
        if (c == 27)
            break;
    }
    while (move_down){
        //check for killswitch
        std::string cmd_line = utilities::get_term_input();
        if(cmd_line.find('x') != std::string::npos){
            break;
        }
        altitude = ::get_height(drone_sensors);
        if(altitude <= 1.5 and altitude > .4){
            int x = 0;
            int y = 0;
            int z = 1;
            int yaw = 0;
            float rate = .4;
            offboard->set_velocity_body({(float)x*rate, (float)y*rate, (float)z*rate, (float)yaw*rate});
        }
        else if(altitude <= .4){
            //breakout, main loop calls land
            break;
        }
    }
    cap.release();
    apriltag_detector_destroy(td);
    //stop killswitch
    utilities::line_buffer(true);
    return true;
}

float april_land::rate_calculator(float altitude){
    if(altitude > 10.0){
        return .5;
    }
    else{
        return .3;
    }
}

int april_land::april_mover_y(int frame_y, double april_y, int distance){
    int y;
    int y_distance_between = frame_y - april_y;

    if(y_distance_between < -distance){
        y = -1;
    }
    else if(y_distance_between > distance){
        y = 1;
    }
    else{
        y = 0;
    }
    return y;
}

int april_land::april_mover_x(int frame_x, double april_x, int distance){
    int x;
    int x_distance_between = frame_x - april_x;

    if(x_distance_between < -distance){
        x = 1;
    }
    else if(x_distance_between > distance){
        x = -1;
    }
    else{
        x = 0;
    }
    return x;
}

int april_land::april_mover_yaw(double yaw_percentage, int x, int y){
    int yaw_change;
    if(yaw_percentage >= 20 && yaw_percentage <= 180 && x == 0 && y == 0){
        yaw_change = -1;
    }
    else if(yaw_percentage >= -180 && yaw_percentage <= -20 && x == 0 && y == 0){
        yaw_change = 1;
    }
    else{
        yaw_change = 0;
    }
    return yaw_change;
}

int april_land::april_mover_z(int x, int y, float altitude){
    int z;
    if(x == 0 && y == 0 && altitude > 1.5){
        z = 1;
    }
    else{
        z = 0;
    }
    return z;
}

double april_land::april_timer(float altitude, int x, int y, int yaw_change){
    time_t mytime;

    std::cout << "x: " << x << std::endl;
    std::cout << "y: " << y << std::endl;
    std::cout << "yaw change: " << yaw_change << std::endl;
    std::cout << "altitude: " << altitude << std::endl;
    if(x == 0 && y == 0 && yaw_change == 0 && altitude < 2.0){
        mytime = time(NULL);
    }
    else{
        mytime = 0;
    }
    return mytime;
}