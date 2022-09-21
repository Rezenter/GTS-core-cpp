//
// Created by ts_group on 6/28/2020.
//

#ifndef CAEN743_FASTSYSTEM_H
#define CAEN743_FASTSYSTEM_H

#include "Crate.h"
#include "Storage.h"
#include "Chatter.h"
#include "Stoppable.h"

class FastSystem : public Stoppable{
private:
    Crate* crate;
    Storage* storage;
    Chatter* chatter;
    Config* config;
    bool exit = true;
    bool payload() override;
    Json messagePayload;
    bool armed = false;

    bool arm();
    bool disarm();
    bool isAlive();

    bool init();

public:
    FastSystem();
    ~FastSystem();

    std::string requestHandler(Json payload);
};


#endif //CAEN743_FASTSYSTEM_H
