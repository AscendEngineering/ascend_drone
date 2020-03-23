#include "manual_control.h"

#include <curses.h>
#include <iostream>
#include <mavsdk/plugins/offboard/offboard.h>
#include <stdlib.h>

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

void translateKeyPress(char key, float& forward, float& right, float& down, float& yaw_right, float& rate){

    if(key=='w'){ //forward
        if(forward <=0){forward += 1;}
    }
    else if(key=='s'){//backward
        if(forward >= 0){forward -= 1;}
    }
    else if(key=='a'){// yaw left
        if(yaw_right >= 0){yaw_right -= 1;}
    }
    else if(key=='d'){//yaw right
        if(yaw_right <=0){yaw_right += 1;}
    }
    else if(key=='q'){//move left
        if(right >= 0){right -= 1;}
    }
    else if(key=='e'){//move right
        if(right <=0){right += 1;}
    }
    else if(key=='r'){//rise (higher)
        if(down >= 0){down -= 1;}
    }
    else if(key=='f'){//fall (lower)
        if(down <=0){down += 1;}
    }
    else if(key=='t'){//rate increase
        rate += 0.5;
    }
    else if(key=='g'){//rate decrease
        rate -= 0.5;
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
        //sent first so it is hit everytime
        offboard->set_velocity_body({forward*rate,right*rate,down*rate,yaw_right*rate});

        char key = getKeyPress();
        if(key==' '){
            continue;
        }
        else if(key=='x'){
            break;
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

