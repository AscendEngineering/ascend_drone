#pragma once

static const int package_pin = 0;

class package_control{

    public:
        package_control();
        void flip_switch(); //flip magnet
        void pickup();      //turn magnet on
        void release();     //turn magnet off


    private:
        bool magnet_on;

};







