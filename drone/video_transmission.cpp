#include "video_transmission.h"

#include <algorithm>  
#include "ascend_zmq.h"
#include "constants.h"
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <zmq.hpp>

//for timing
#include <chrono>
#include <ctime>
#include <thread>


// video_transmission::video_transmission(const std::string worker_address){
//     this->worker_address=worker_address;
// }

// void video_transmission::start_transmission(){
//     std::cout<<"TODO: start transmission"<<std::endl;
	    
//     //Camera.set(cv::CAP_PROP_EXPOSURE,0);
//     Camera.set(cv::CAP_PROP_FPS,0);
//     Camera.set(cv::CAP_PROP_MODE,0);
//     if(!Camera.open()){
//         //TODO throw error
//         std::cerr <<"Error opening camera" << std::endl;
//         return;
//     }
    
//     std::this_thread::sleep_for(std::chrono::milliseconds(3000));

//     std::cout<<"Connected to camera ="<<Camera.getId() << std::endl;
//     cv::Mat image;
   
//     //buffer for later
//     std::string encoded_img;
//     encoded_img.reserve(3686400); //rows*cols
    
//     const auto p1 = std::chrono::system_clock::now();

// 	 unsigned int iterations = 100;
//     for(unsigned int i=0; i< iterations; i++){
//     //while(true){
//         Camera.grab();
//         Camera.retrieve ( image );
        
//         //std::string imgData(image.datastart,image.dataend);

//         //encode and base64
//         //cv::imencode(".jpg",image,buf);
        
//         //base64_encode(image.data,3686400,encoded_img);

//         //send over
//         //bool succ = comm::send_msg(send_socket,"drone1",encoded_img,"tcp://localhost:"+constants::from_drone);

// 		  //cv::namedWindow( "Image", cv::WINDOW_AUTOSIZE );
//         //cv::imshow("Image", image);
//         //cv::waitKey(0);

//     }
//     const auto p2 = std::chrono::system_clock::now();
//     auto framesPerSecond = ((double)(1000*iterations)/(double)std::chrono::duration_cast<std::chrono::milliseconds>(p2-p1).count());
//     std::cout<< "Frames per second: " << 
// 	 framesPerSecond
// 	 <<std::endl;


// }

// void video_transmission::stop_transmission(){
    
//     std::cout<<"TODO: stop transmission"<<std::endl;

// }



video_transmission::video_transmission(const std::string worker_address){
    this->worker_address=worker_address;
}



void video_transmission::start_transmission(){
    std::cout<<"TODO: start transmission"<<std::endl;
}




void video_transmission::stop_transmission(){
    
    std::cout<<"TODO: stop transmission"<<std::endl;

}
