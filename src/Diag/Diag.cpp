//
// Created by ts_group on 23.12.2022.
//
#include "include/Diag/Diag.h"
#include <iostream>
#include <utility>

Diag::Diag(): storage(*this) {
    try{
        s = new http::server3::server(config.IP, config.Port, config.HTMLRoot, config.ServerTheadCount, [this](Json req) -> Json {
            return this->APIRequestHandler(std::move(req));
        });
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
    Json resp;
    try {
        if (req.at("subsystem") == "diag") {
            if (req.at("reqtype") == "get_configs") {
                resp = storage.getConfigsNames();
                resp["ok"] = true;
            } else if (req.at("reqtype") == "load_config") {
                resp = storage.loadConfig(req.at("config"));
            } else {
                resp = {{"ok",  false},
                        {"err", "unknown reqtype"}};
            }
            //resp = crate.requestHandler(req);
        } else if (req.at("subsystem") == "ADC") {
            //resp = crate.requestHandler(req);
        } else if (req.at("subsystem") == "slowADC") {
            //resp = slowSubsystem.requestHandler(req);
        } else {
            resp = {{"ok",  false},
                    {"err", "unknown subsystem"}};
        }
    }catch (const std::exception& e) {
        resp = {{"ok",  false},
                {"err", e.what()}};
    }
    return resp;
}