#pragma once

static const int package_pin = 0;

class package_control{

    public:
        static package_control& get_instance(){
            static package_control control_instance;
            return control_instance;
        }
        void flip_switch(); //flip magnet
        void pickup();      //turn magnet on
        void release();     //turn magnet off

        //turn off functions
        package_control(package_control const&) = delete;
        void operator=(package_control const&) = delete;


    private:
        bool magnet_on;
        package_control();
        

};







