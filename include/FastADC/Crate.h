//
// Created by ts_group on 6/28/2020.
//

#ifndef CAEN743_CRATE_H
#define CAEN743_CRATE_H

#include "Link.h"

#include "Config.h"
#include "Storage.h"

union Buffer{
    unsigned short int val[2];
    char chars[4];
};

class Crate : public Stoppable{
private:
    Buffer buffer;
    Config config;
    Storage storage;
    Link* links[MAX_LINKS];

    bool payload() override;
    void beforePayload() override;
    void afterPayload() override;

    SOCKET sockfd;
    struct sockaddr_in servaddr;
    size_t currentEvent = 0;
    std::array<std::latch*, SHOT_COUNT> processed;

    std::array<unsigned short , SHOT_COUNT> DAC1;
    std::array<unsigned short , SHOT_COUNT> DAC2;
    double expectedNePerShot[SHOT_COUNT];
    double measuredNeShot[SHOT_COUNT];
    double neError[SHOT_COUNT];
    double ph_el[SHOT_COUNT];
    double p_coeff = 0.0;
    const double valveDeadzone = 1.2; //[V] valve does not react to the voltage lover than this level
    size_t plasmaLock = 0;
    const size_t plasmaLockIndex = 40;
    const double plasmaLockThreshold = 1e18;

    //state
    bool armed = false;
    //size_t recordedCount = 0; //current laser pulse
    size_t connectedCount = 0;
    size_t boardCount = 0; //all boards
    bool saving = false; //saving
    bool saved = true; //saved
    bool joinMe = false;

public:
    ~Crate() override;
    Crate();

    Json requestHandler(Json req);

    bool arm();
    bool disarm();
    bool isAlive();
    bool init();
    Json result;
};


#endif //CAEN743_CRATE_H
