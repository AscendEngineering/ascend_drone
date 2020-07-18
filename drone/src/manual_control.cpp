#include "manual_control.h"

#include <curses.h>
#include <iostream>
#include <mavsdk/plugins/offboard/offboard.h>
#include <stdlib.h>
#include <chrono> 
#include <thread> 

char getKeyPress(){

    char cmd;
    int keypress = getch();

    if(keypress == ERR){
        return ' ';
    }
    
    //keypress translate
    if(keypress == 27 && getch() == 91){
        keypress = getch();
        if(keypress==65){ cmd='r';}
        else if(keypress==66){ cmd='f';}
    }
    else{
        cmd = (char)keypress;
    }

    return cmd;
}


void manual_control::translateKeyPress(char key, float& forward, float& right, float& down, float& yaw_right, float& rate){

    if(key=='w'){ //forward
        if(forward <=0){forward += HORIZONTAL_INCREMENTS;}
    }
    else if(key=='s'){//backward
        if(forward >= 0){forward -= HORIZONTAL_INCREMENTS;}
    }
    else if(key=='a'){// yaw left
        if(yaw_right >= 0){yaw_right -= YAW_INCREMENTS;}
    }
    else if(key=='d'){//yaw right
        if(yaw_right <=0){yaw_right += YAW_INCREMENTS;}
    }
    else if(key=='q'){//move left
        if(right >= 0){right -= HORIZONTAL_INCREMENTS;}
    }
    else if(key=='e'){//move right
        if(right <=0){right += HORIZONTAL_INCREMENTS;}
    }
    else if(key=='r'){//rise (higher)
        if(down >= 0){down -= VERTICAL_INCREMENTS;}
    }
    else if(key=='f'){//fall (lower)
        if(down <=0){down += VERTICAL_INCREMENTS;}
    }
    else if(key=='t'){//rate increase
        rate += 0.1;
    }
    else if(key=='g'){//rate decrease
        rate -= 0.1;
    }
    else if(key=='0'){//reset all
        forward=0;yaw_right=0;right=0;down=0;
    }
    else if(key=='p'){//package
        package.flip_switch();
    }
    else{
        refresh();
    }
}

manual_control::manual_control(System* system){

    //setup flight variables
    float forward = 0.0;
    float right = 0.0;
    float down = 0.0;
    float yaw_right = 0.0;
    float rate = 1.0;

    //enter manual mode
    auto offboard = std::make_shared<Offboard>(*system);
    offboard->set_velocity_body({forward, right, down, yaw_right}); /* Needed */
    Offboard::Result offboard_result = offboard->start();

    //error check
    if(offboard_result != Offboard::Result::SUCCESS){
        std::cerr << "Error gaining offboard control" << std::endl;
    }

    //start ncurses
    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    scrollok(stdscr,TRUE);
    clear();

    while(true){

        char key = getKeyPress();
        if(key==' '){
            continue;
        }
        else if(key=='x'){
            break;
        }
        else if(key=='l'){ //launch
            //manual_takeoff(system);
        }
        else if(key=='k'){ //kill
            //manual_land(system);
        }

        translateKeyPress(key,forward,right,down,yaw_right,rate);
        std::string msg = ("forward: " + std::to_string(forward) + 
            ", right: " + std::to_string(right) +
            ", down: " + std::to_string(down) + 
            ", yaw_right: " + std::to_string(yaw_right) +
            ", rate: " + std::to_string(rate) + 
            "\n").c_str();
        for(int i =0; i< msg.size(); i++){
            addch(msg[i]);
        }

        offboard->set_velocity_body({forward*rate,right*rate,down*rate,yaw_right*rate});
        
        refresh();
    }
    endwin();

    //stop offboard mode
    offboard->set_velocity_body({forward,right,down,yaw_right});
    offboard_result = offboard->stop();
    
    //error check
    if(offboard_result != Offboard::Result::SUCCESS){
        std::cerr << "Error stopping offboard control" << std::endl;
    }
}

// void manual_control::manual_takeoff(std::shared_ptr<mavsdk::Action> action){
//     std::shared_ptr<mavsdk::Action> action = std::make_shared<Action>(*system);
//     std::shared_ptr<mavsdk::Telemetry> telemetry = std::make_shared<Telemetry>(*system);
//     //check our health
//     while (telemetry->health_all_ok() != true) {
//         Telemetry::Health health = telemetry->health();
//         std::cerr << "Drone is not healthy: " << health << std::endl;
//         std::this_thread::sleep_for(std::chrono::seconds(1));
//     }

//     //arm
//     const Action::Result arm_result = action->arm();
//     std::this_thread::sleep_for(std::chrono::seconds(5));

//     //takeoff
//     action->set_takeoff_altitude(5);
//     const Action::Result takeoff_result = action->takeoff();
//     std::this_thread::sleep_for(std::chrono::seconds(5));
// }

// void manual_control::manual_land(std::shared_ptr<mavsdk::Action> action){
//     const Action::Result land_result = action->land();

// }
