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
                                      int lng,
                                      int lat, 
                                      int alt, 
                                      int bat_percentage);

        std::string serialize(const ascend::msg& to_send);
        ascend::msg deserialize(const std::string& recv);

}