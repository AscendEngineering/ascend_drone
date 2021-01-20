#include "april_land.h"
#include <sstream>
#include <stdio.h>
#include <thread> 
#include <chrono>  

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


bool april_land::execute(std::shared_ptr<Offboard> offboard,std::shared_ptr<px4_sensors> drone_sensors){

    //offboard should already be initiated
    if(!offboard->is_active()){
        return false;
    }

    //kill any ffmpeg process to server
    FILE* kill_ffmpeg = popen("pkill -f ffmpeg","r");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Initialize camera
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Couldn't open video capture device" << std::endl;
        return -1;
    }

    double altitude = ::get_height(drone_sensors);
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
            //printf("%f %f moveto %f %f lineto %f %f lineto %f %f lineto %f %f lineto stroke\n",
            //        det->p[0][0], det->p[0][1],
            //        det->p[1][0], det->p[1][1],
            //        det->p[2][0], det->p[2][1],
            //        det->p[3][0], det->p[3][1],
            //        det->p[0][0], det->p[0][1]);

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
        
        if(zarray_size(detections) > 0){
            apriltag_detection_t *det_final;
            zarray_get(detections, 0, &det_final);            
            altitude = ::get_height(drone_sensors);
            float rate = rate_calculator(altitude);

            //april center calculation
            LOG_S(INFO)<< "X CENTER OF APRIL: " << det_final->c[0];
            LOG_S(INFO)<< "Y CENTER OF APRIL: " << det_final->c[1];          
            //distance in pixels
            int distance = 150;
            
            //X and Y VALUES DECLARED HERE
            int x = april_mover_x(frame_x_center, det_final->c[0], distance);
            int y = april_mover_y(frame_y_center, det_final->c[1], distance);
            //****************************

            LOG_S(INFO)<< "X integer: " << x;
            LOG_S(INFO)<< "Y integer: " << y;

            apriltag_detection_info_t info;
            info.tagsize = .1;
            info.fx = 2191.82299;
            info.fy = 2145.07083;
            info.cx = 321.340863;
            info.cy = 174.398525;
            info.det = det_final;
                    
            apriltag_pose_t pose;
            double err = estimate_tag_pose(&info, &pose);
            double pi = 3.14159;
            double val = 180.0 / pi;

            double x1 = pose.R->data[0];
            double y1 = pose.R->data[1];
            double z1 = pose.R->data[2];

            double x2 = pose.R->data[3];
            double y2 = pose.R->data[4];
            double z2 = pose.R->data[5];

            double x3 = pose.R->data[6];
            double y3 = pose.R->data[7];
            double z3 = pose.R->data[8];

            double roll = atan2(-z2, z3) * val;
            double pitch = asin(z1) * val;
            double yaw_percentage = atan2(-y1, x1) * val;
            LOG_S(INFO)<< "Yaw val: " << yaw_percentage; 

            //YAW VALUE DECLARED HERE
            int yaw = april_mover_yaw(yaw_percentage, x, y);
            //**********************

            
            LOG_S(INFO)<< "Yaw integer: " << yaw;
            LOG_S(INFO)<< "Altitude: " << altitude;
            LOG_S(INFO)<< "Rate: " << rate;

            //Z VALUE DECLARED HERE            
            int z = april_mover_z(x, y, altitude);
            float real_z = (float)z;
            if((real_z*rate)==0.0){
                real_z = 0.2;
            }
            //*********************

            //call the movement here
            offboard->set_velocity_body({(float)x*rate, (float)y*rate, real_z, ::adjust_yaw(yaw,rate)});

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


            if(x != 0 && y != 0 && altitude > 1.5){
                timer_differential = 0;
            }
            LOG_S(INFO)<< "Z: " << real_z;
            LOG_S(INFO)<< "Timer Differential: " << timer_differential << "\n";
        }
        apriltag_detections_destroy(detections);
        
        imshow("Tag Detections", frame);
        char c = (char)waitKey(1);
        if (c == 27)
            break;
    }

    while (move_down){
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

    return true;
}

int april_land::rate_calculator(float altitude){
    if(altitude > 10.0){
        return .5;
    }
    else{
        return .2;
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
    if(x == 0 && y == 0 && yaw_change == 0 && altitude < 1.5){
        mytime = time(NULL);
    }
    else{
        mytime = 0;
    }
    return mytime;
}