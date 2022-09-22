//
// Created by ts_group on 6/28/2020.
//

#ifndef CAEN743_FASTSYSTEM_H
#define CAEN743_FASTSYSTEM_H

#include "Crate.h"
#include "Storage.h"
#include "Stoppable.h"

class FastSystem{
private:
    Crate* crate;
    Storage* storage;
    Config* config;
    bool ready = false;
    bool armed = false;

    bool arm();
    bool disarm();
    bool isAlive();

    bool init();
    void kill();

public:
    FastSystem();
    ~FastSystem();

    Json requestHandler(Json req);
};


#endif //CAEN743_FASTSYSTEM_H
