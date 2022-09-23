//
// Created by ts_group on 6/28/2020.
//

#include "include/FastADC/FastSystem.h"

bool FastSystem::arm() {
    if(!this->armed){
        this->armed = crate->arm();
        return this->armed;
    }
    return true;
}

bool FastSystem::disarm() {
    if(this->armed){
        this->armed = false;
        return storage->saveDischarge(crate->disarm());
    }
    return true;
}

bool FastSystem::isAlive() {
    if(!ready){
        return false;
    }
    if(!crate->isAlive()){
        return false;
    }
    if(!storage->isAlive()){
        return false;
    }
    return true;
}

bool FastSystem::init() {
    this->ready = false;
    this->config = new Config();
    if(!this->config->load()){
        std::cout << "something went wrong during loading config." << std::endl;
    }
    std::cout << "Voltage range: [" << this->config->offset - 1250 << ", " << this->config->offset + 1250 << "] mv." << std::endl;
    //std::cout << "Trigger level = " << config.triggerThreshold << " mv." << std::endl; // ch0 trigger should not be used

    this->crate = new Crate(*this->config);
    this->storage = new Storage(*this->config);
    this->ready = true;
    if(!isAlive()){
        std::cout << "FastSystem init failed" << std::endl;
        this->kill();
        return false;
    }
    this->armed = false;
    std::cout << "System initialised" <<std::endl;
    return true;
}

void FastSystem::kill() {
    std::cout << "fastSystem destructor... ";
    if(this->ready) {
        delete this->crate;
        delete this->storage;
        delete this->config;
        this->ready = false;
        this->armed = false;
    }
    std::cout << "OK" << std::endl;
}

FastSystem::~FastSystem() {
    this->kill();
}

FastSystem::FastSystem() {

}

Json FastSystem::requestHandler(Json req){
    Json resp = {{"ok", true}};
    if(req.contains("reqtype")){
        if(req.at("reqtype") == "reboot"){
            if(this->ready){
                this->kill();
            }
            if(!this->init()){
                resp["ok"] = false;
                resp["err"] = "internal fail";
            }
        }else
            if(req.at("reqtype") == "arm"){
                if(!isAlive()){
                    resp["ok"] = false;
                    resp["err"] = "Fast system is dead.";
                }else {
                    if (req.contains("header")) {
                        config->aux_args = req["header"];
                    } else {
                        config->aux_args = {};
                    }
                    if (!this->arm()) {
                        resp["ok"] = false;
                        resp["err"] = "internal fail";
                    }
                }
        }else
            if(req.at("reqtype") == "disarm"){
                if(!this->ready){
                    resp["ok"] = false;
                    resp["err"] = "system is not initialised!";
                }else
                    if(!this->isAlive()){
                        resp["ok"] = false;
                        resp["err"] = "system is dead!";
                    }else{
                        if(!this->disarm()){
                            resp["ok"] = false;
                            resp["err"] = "internal fail";
                        }
                    }
        }else
            if(req.at("reqtype") == "state"){
                resp["alive"] = this->isAlive();
                resp["armed"] = this->armed;
                if(resp["alive"]) {
                    resp["plasma_shotn"] = this->config->plasmaShot;
                    resp["debug_shotn"] = this->config->debugShot;
                }else{
                    resp["plasma_shotn"] = 99999;
                    resp["debug_shotn"] = 99999;
                }
        }
    }else{
        resp["ok"] = false;
        resp["err"] = "reqtype is not listed";
    }
    return resp;
}