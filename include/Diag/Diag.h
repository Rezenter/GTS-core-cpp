//
// Created by ts_group on 23.12.2022.
//

#ifndef GTS_CORE_DIAG_H
#define GTS_CORE_DIAG_H
//in-out
#include "Server/server.hpp"

//subsystems
#include "include/Diag/Config.h"
//#include "include/FastADC/Crate.h"
//#include "include/SlowADC/SlowSubsystem.h"

class Diag{
private:
    http::server3::server* s;

    //Crate crate;
    //SlowSubsystem slowSubsystem;


public:
    Config config;

    Diag();
    ~Diag();


    static Json APIRequestHandler(Json req);
};

#endif //GTS_CORE_DIAG_H
