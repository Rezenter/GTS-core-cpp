//
// Created by ts_group on 23.12.2022.
//
#include "include/Diag/Diag.h"
#include <iostream>

//Diag::Diag(): storage(*this) {
Diag::Diag() {
    try{
        s = new http::server3::server(config.IP, config.Port, config.HTMLRoot, config.ServerTheadCount, APIRequestHandler);
        s->run();
    }
    catch (std::exception& e){
        std::cerr << "Diag.cpp exception: " << e.what() << "\n";
    }
}

Diag::~Diag() {
    //stop server
    delete s;
}

Json Diag::APIRequestHandler(Json req){
    Json resp = {{"ok", false},
                 {"err", "unknown subsystem"}};
    if(req.at("subsystem") == "diag") {
        if(req.at("reqtype") == "get_configs"){
            //resp["data"] = this->storage.getConfigsNames();
        }
        resp = {{"ok", true}};
        //resp = crate.requestHandler(req);
    }else if(req.at("subsystem") == "ADC"){
        //resp = crate.requestHandler(req);
    }else if(req.at("subsystem") == "slowADC"){
        //resp = slowSubsystem.requestHandler(req);
    }
    return resp;
}