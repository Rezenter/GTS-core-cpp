//
// Created by ts_group on 6/28/2020.
//

#ifndef CAEN743_CRATE_H
#define CAEN743_CRATE_H

#include "CAEN743.h"
#include "common.h"


class Crate{
private:
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
