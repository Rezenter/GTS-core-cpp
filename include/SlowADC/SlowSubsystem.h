//
// Created by ts_group on 20.12.2022.
//

#ifndef GTS_CORE_SLOWSUBSYSTEM_H
#define GTS_CORE_SLOWSUBSYSTEM_H

#include "json.hpp"
#include <string>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>

#define PORT "3425"
#define USER "root"
#define PASS "terasic"
#define MAX_LENGTH 128

using Json = nlohmann::json;
using boost::asio::ip::tcp;

class SlowSubsystem {
private:
    const static inline char req_status[] = "\x0c";
    const static inline char req_arm_10V[] = "\x02\x05\x00\x2c\x02";
    const static inline char req_arm_5V[] = "\x02\x05\x01\x2c\x02";

    const static inline char status_ready[] = "\x00";
    const static inline char status_busy[] = "\x4b";

    const static inline std::string addresses[4] = {"192.168.10.50", "192.168.10.51", "192.168.10.52", "192.168.10.53"};
    boost::asio::io_context io_context;

    bool connect();
public:
    ~SlowSubsystem();
    SlowSubsystem();

    Json requestHandler(Json req);
};


#endif //GTS_CORE_SLOWSUBSYSTEM_H
