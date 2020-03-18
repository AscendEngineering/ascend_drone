#include "manual_control.h"

#include <curses.h>
#include <iostream>
#include <mavsdk/plugins/offboard/offboard.h>
#include <stdlib.h>

char getKeyPress(){

    char cmd;

    int keypress = getch();
    
    //keypress translate
    if(keypress == 27 && getch() == 91){
        keypress = getch();
        if(keypress==65){ cmd='w';}
        else if(keypress==66){ cmd='s';}
        else if(keypress==67){ cmd='d';}
        else if(keypress==68){ cmd='a';}
    }
    else{
        cmd = (char)keypress;
    }

    return cmd;

}


manual_control::manual_control(System* system){

    //enter manual mode
    auto offboard = std::make_shared<Offboard>(*system);
    offboard->set_velocity_body({0.0f, 0.0f, 0.0f, 0.0f}); /* Needed */
    Offboard::Result offboard_result = offboard->start();

    //error check
    if(offboard_result != Offboard::Result::SUCCESS){
        std::cerr << "Errror gaining offboard control" << std::endl;
    }

    //start ncurses
    initscr();
    cbreak();
    noecho();
    clear();

    while(true){
        char key = getKeyPress();


        if(key=='q'){ //quit
            break;
        }
        else if(key=='w'){ //forward
            offboard->set_velocity_body({0.1f, 0.0f, 0.0f, 0.0f});
        }
        else if(key=='s'){//backward
            offboard->set_velocity_body({-0.1f, 0.0f, 0.0f, 0.0f});
        }
        else if(key=='a'){//left
            offboard->set_velocity_body({-0.1f, 0.0f, 0.0f, -0.1f});
        }
        else if(key=='d'){//right
            offboard->set_velocity_body({-0.1f, 0.0f, 0.0f, 0.1f});
        }

    }
    endwin();

    //stop offboard mode
    offboard->set_velocity_body({0.0f, 0.0f, 0.0f, 0.0f});
    offboard_result = offboard->stop();
    
    //error check
    if(offboard_result != Offboard::Result::SUCCESS){
        std::cerr << "Errror gaining offboard control" << std::endl;
    }
}

