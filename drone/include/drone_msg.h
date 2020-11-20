#pragma once

#include "msgDef.pb.h"
#include <string>

enum drone_status {
  AVAILABLE,
  IN_USE,
  REPAIR,
  RETIRED
};


namespace msg_generator {

        std::string generate_heartbeat(const std::string& name, 
                                      double lng,
                                      double lat, 
                                      double alt, 
                                      double bat_percentage);
        std::string generate_land_request(const std::string& drone_name);

        std::string serialize(const ascend::msg& to_send);
        ascend::msg deserialize(const std::string& recv);

}