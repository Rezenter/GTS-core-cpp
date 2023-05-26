//
// Created by ts_group on 23.12.2022.
//

#ifndef DIAG_H
#define DIAG_H
//in-out
#include "Server/server.hpp"

//subsystems
#include "include/Diag/Config.h"
#include "include/Diag/Storage.h"
//#include "include/FastADC/Crate.h"
//#include "include/SlowADC/SlowSubsystem.h"

class Diag{
private:
    http::server3::server* s;

    //Crate crate;
    //SlowSubsystem slowSubsystem;


public:
    Config config;
    Storage storage;

    Diag();
    ~Diag();

    Json APIRequestHandler(Json req);
};

#endif //DIAG_H
