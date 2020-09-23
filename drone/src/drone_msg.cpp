#include "drone_msg.h"


std::string msg_generator::generate_heartbeat(const std::string& drone_name, 
                                            double lng, 
                                            double lat, 
                                            double alt, 
                                            double bat_percentage){
    ascend::msg msg;
    msg.set_name(drone_name);

    ascend::heartbeat_msg heartbeat;
    heartbeat.set_lng(lng);
    heartbeat.set_lat(lat);
    heartbeat.set_alt(alt);
    heartbeat.set_bat_percentage(bat_percentage);
    *msg.mutable_heartbeat() = heartbeat;

    return serialize(msg);
}


std::string msg_generator::serialize(const ascend::msg& to_send){

    //form to a string
    std::string serial_msg; 
    to_send.SerializeToString(&serial_msg);

    return serial_msg;
}


ascend::msg msg_generator::deserialize(const std::string& recv){
    ascend::msg retval;
    retval.ParseFromString(recv);

    return retval;


}





