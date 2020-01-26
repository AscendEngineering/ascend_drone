#pragma once

#include "status.pb.h"
#include <string>

namespace msg_generator {

        std::string generate_heartbeat(int lng, int lat, int alt, int bat_percentage);
        std::string generate_emergency(const std::string& drone_name);
        std::string generate_land_request(const std::string& drone_name);

        std::string serialize(const ascend::msg& to_send);
        ascend::msg deserialize(const std::string& recv);

}