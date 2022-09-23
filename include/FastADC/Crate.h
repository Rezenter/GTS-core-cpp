//
// Created by ts_group on 6/28/2020.
//

#ifndef CAEN743_CRATE_H
#define CAEN743_CRATE_H

#include "CAEN743.h"
#include "Config.h"

union Buffer{
    unsigned short int val;
    char chars[2];
};

class Crate : public Stoppable{
private:
    Buffer buffer;
    Config& config;
    CAEN743* caens[MAX_CAENS];
    bool online = false;

public:
    explicit Crate(Config& config);
    bool arm();
    Json disarm();
    bool isAlive();

    bool init();
};


#endif //CAEN743_CRATE_H
