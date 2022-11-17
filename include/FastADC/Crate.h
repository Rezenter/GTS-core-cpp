//
// Created by ts_group on 6/28/2020.
//

#ifndef CAEN743_CRATE_H
#define CAEN743_CRATE_H

#include "Link.h"
#include "Processor.h"
#include "Config.h"

union Buffer{
    unsigned short int val;
    char chars[2];
};

class Crate : public Stoppable{
private:
    Buffer buffer;
    Config& config;
    Link* links[MAX_LINKS];
    Processor* processors[MAX_CAENS];

    bool payload() override;
    void beforePayload() override;
    void afterPayload() override;

    SOCKET sockfd;
    struct sockaddr_in servaddr;
    size_t currentEvent = 0;
    bool armed = false;
    std::array<unsigned short , SHOT_COUNT> DAC1;
    std::array<std::latch*, SHOT_COUNT> processed;
    std::array<std::chrono::time_point<std::chrono::steady_clock>, SHOT_COUNT> tProcessedAll;
    std::array<std::chrono::time_point<std::chrono::steady_clock>, SHOT_COUNT> tDACSend;
    std::chrono::time_point<std::chrono::steady_clock> tArm;

public:
    ~Crate() override;
    explicit Crate(Config& config);
    bool arm();
    Json disarm();
    bool isAlive();

    bool init();
};


#endif //CAEN743_CRATE_H
