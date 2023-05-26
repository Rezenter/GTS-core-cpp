//
// Created by ts_group on 6/28/2020.
//

#include "include/FastADC/Crate.h"

//debug
#include <iostream>
//debug


Crate::Crate(){
    if(!this->config.load()){
        std::cout << "something went wrong during loading config." << std::endl;
    }
    std::cout << "Voltage range: [" << this->config.offset - 1250 << ", " << this->config.offset + 1250 << "] mv." << std::endl;
    this->storage = Storage_old(&this->config);
    init();
}

Crate::~Crate(){
    disarm();
    for(unsigned int count = 0; count < config.linkCount; count++){
        delete links[count];
    }
}

Json Crate::requestHandler(Json req){
    Json resp = {{"ok", true}};
    if(req.contains("reqtype")){
        if(req.at("reqtype") == "reboot"){

            resp["ok"] = false;
            resp["err"] = "Not implemented!";

        }else
        if(req.at("reqtype") == "arm"){
            if(!isAlive()){
                resp["ok"] = false;
                resp["err"] = "Fast system is dead.";
            }else {
                if (req.contains("header")) {
                    config.aux_args = req["header"];
                } else {
                    config.aux_args = {};
                }
                if (req.contains("is_plasma")) {
                    config.isPlasma = req.at("is_plasma");
                }else{
                    config.isPlasma = false;
                }
                if (!this->arm()) {
                    resp["ok"] = false;
                    resp["err"] = "internal fail";
                }
            }
        }else
        if(req.at("reqtype") == "disarm"){
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
            resp["CAENS"] = {
                    {"alive", this->isAlive()},
                    {"armed", this->armed},
                    {"saving", this->saving},
                    {"saved", this->saved},
                    {"recordedPulses", this->currentEvent},
                    {"connectedCount", this->connectedCount},
                    {"boardCount", this->boardCount}
            };

            if(resp["CAENS"]["alive"]) {
                resp["plasma_shotn"] = this->config.plasmaShot;
                resp["debug_shotn"] = this->config.debugShot;
            }else{
                resp["plasma_shotn"] = 99999;
                resp["debug_shotn"] = 99999;
            }
        }else
        if(req.at("reqtype") == "configs"){
            resp["data"] = this->storage.getConfigsNames();
        }else
        if(req.at("reqtype") == "getGas"){
            return this->storage.getGas(req.at("name"));
        }else
        if(req.at("reqtype") == "saveGas"){
            resp["ok"] = this->storage.saveGas(req.at("name"), req.at("prog"));
            return resp;
        }
    }else{
        resp["ok"] = false;
        resp["err"] = "reqtype is not listed";
    }
    return resp;
}

bool Crate::init() {
    armed = false;
    connectedCount = 0;
    boardCount = config.linkCount;
    saving = false;
    for(unsigned int link = 0; link < config.linkCount; link++){
        std::cout << link << std::endl;
        links[link] = new Link(config.links[link], processed);
        links[link]->init(config);
        if(!links[link]->isAlive()){
            std::cout << "board " << link << " not initialised!" << std::endl;
            return false;
        }
        connectedCount++;
    }
    std::cout << "Fast digitizers initialised!" << std::endl;
    saved = true;
    return true;
}

bool Crate::arm() {
    if(!saved){
        return false;
    }

    std::cout << "loading gas program..." << std::endl;

    Json prog = storage.getGas(config.aux_args["ne_program"]);
    if(!prog["ok"]){
        std::cout << "\n\nBAD gas program! NOT armed!\n\n" << std::endl;
        for(size_t eventInd = 1; eventInd < SHOT_COUNT; eventInd++) {
            expectedNePerShot[eventInd] = 0.0;
            measuredNeShot[eventInd] = 0.0;
            neError[eventInd] = 0.0;
            ph_el[eventInd] = 0.0;
        }
        return false;
    }
    expectedNePerShot[0] = 0.0;
    measuredNeShot[0] = 0.0;
    neError[0] = 0.0;
    ph_el[0] = 0.0;
    const float firstShotTime = 3.7;  // [ms]
    const float laserPeriod = 3.127;  // [ms] (increase frequency on 10Hz)
    size_t current_ind = 0;
    for(size_t eventInd = 1; eventInd < SHOT_COUNT; eventInd++) {
        float time = firstShotTime + (float)(eventInd - 1) * laserPeriod;
        while(current_ind < prog["data"]["desired_ne"].size() && prog["data"]["desired_ne"][current_ind]["time"] < time){
            current_ind ++;
        }
        expectedNePerShot[eventInd] = (double)prog["data"]["desired_ne"][current_ind - 1]["ne"] + ((double)prog["data"]["desired_ne"][current_ind]["ne"] - (double)prog["data"]["desired_ne"][current_ind - 1]["ne"]) * (time - (double)prog["data"]["desired_ne"][current_ind - 1]["time"]) / ((double)prog["data"]["desired_ne"][current_ind]["time"] - (double)prog["data"]["desired_ne"][current_ind - 1]["time"]);
        measuredNeShot[eventInd] = 0.0;
        neError[eventInd] = 0.0;
        ph_el[eventInd] = 0.0;
    }
    p_coeff = (double)prog["data"]["valve_coeff"];
    std::cout << "done" << std::endl;

    for(unsigned int count = 0; count < config.linkCount; count++){
        if(!links[count]->arm()) {
            return false;
        }
    }
    armed = true;
    saving = false;
    saved = false;
    associatedThread = std::thread([&](){
        run();
    });
    std::cout << "armed" << std::endl;
    return true;
}

bool Crate::disarm() {
    if(!armed){
        associatedThread.join();
        return false;
    }

    this->requestStop();
    associatedThread.join();
    std::cout << "all joined" << std::endl;
    return true;
}

bool Crate::isAlive() {
    if(joinMe){
        associatedThread.join();
        joinMe = false;
    }
    if(!storage.isAlive()){
        return false;
    }
    for(unsigned int count = 0; count < config.linkCount; count++){
        if(!links[count]->isAlive()){
            std::cout << "board " << count << " dead!" << std::endl;
            return false;
        }
    }
    return true;
}

bool Crate::payload() {
    if(processed[currentEvent]->try_wait()){
        for(size_t ch = 0; ch < 5; ch++){
            ph_el[currentEvent] += links[1]->ph_el[0][currentEvent][ch + 11];
        }
        //ph_el = (currentEvent + 1) * 40.0 * 60;
        ph_el[currentEvent] = fmax(0.0, ph_el[currentEvent]) * 0.023; // FOR 1.6 J ONLY!!!
        measuredNeShot[currentEvent] = ph_el[currentEvent] * 3.54e19 * 5 / 4096;
        neError[currentEvent] = expectedNePerShot[currentEvent] - measuredNeShot[currentEvent];
        ph_el[currentEvent] = fmin(4095, ph_el[currentEvent]);
        buffer.val[0] = floor(ph_el[currentEvent]);
        DAC1[currentEvent] = buffer.val[0];

        if (currentEvent >= plasmaLockIndex && plasmaLock == 0) {
            if (measuredNeShot[currentEvent] < plasmaLockThreshold) {
                plasmaLock = currentEvent;
            }
        }

        if(neError[currentEvent] > 0 && plasmaLock == 0) {
            buffer.val[1] = floor(fmin(3000, fmax(0.0, (valveDeadzone + neError[currentEvent] * p_coeff) * 4096 / 5))); // limit gas puff to 4 volts
        }else{
            buffer.val[1] = 0;
        }

        DAC2[currentEvent] = buffer.val[1];

        currentEvent++;
        sendto(sockfd, buffer.chars, 4,
               0, (const struct sockaddr *) &servaddr,
               sizeof(servaddr));
        return currentEvent == SHOT_COUNT;
    }else{
        std::this_thread::sleep_for(std::chrono::microseconds(10));
        return false;
    }
}

void Crate::beforePayload() {
    DAC1.fill(0);
    for(size_t eventInd = 0; eventInd < SHOT_COUNT; eventInd++) {
        //processed[evenInd] = new std::latch(config.caenCount);
        processed[eventInd] = new std::latch(config.linkCount);
    }

    result = {};
    unsigned long long mask = 1 << 1;
    SetThreadAffinityMask(GetCurrentThread(), mask);
    std::cout << "SlowSubsystem thread: " << SetThreadAffinityMask(GetCurrentThread(), mask) << std::endl;
    currentEvent = 0;
    //open socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&servaddr, 0, sizeof(servaddr));
    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8080);
    servaddr.sin_addr.s_addr = inet_addr("192.168.10.56");

    buffer.val[0] = 0;
    buffer.val[1] = 0;
    plasmaLock = 0;
    sendto(sockfd, buffer.chars, 4,
           0, (const struct sockaddr *) &servaddr,
           sizeof(servaddr));
}

void Crate::afterPayload() {
    std::cout << "crate events: " << currentEvent << std::endl;
    buffer.val[0] = 0;
    buffer.val[1] = 0;
    sendto(sockfd, buffer.chars, 4,
           0, (const struct sockaddr *) &servaddr,
           sizeof(servaddr));
    for(size_t evenInd = 0; evenInd < SHOT_COUNT; evenInd++) {
        delete processed[evenInd];
    }
    //close socket
    closesocket(sockfd);

    armed = false;
    saving = true;
    result = {
            {"header", {
                               {"version", 4},
                               {"eventLength", config.recordLength},
                               {"frequency", config.freqStr()},
                               {"boards", Json::array()},
                               {"offset", config.offset},
                               {"aux", config.aux_args},
                               {"RT", {
                                   {"ph_el",  ph_el},
                                   {"DAC1",  DAC1},
                                   {"DAC2",  DAC2},
                                   {"neProg",  expectedNePerShot},
                                   {"neMeas",  measuredNeShot},
                                   {"neErr",  neError},
                                   {"plasmaLock", plasmaLock}
                               }}
                       }
            },
            {"boards", Json::array()}
    };
    for(size_t board = 0; board < config.linkCount * 2; board++){
        Json boardData = Json::array();

        for (size_t event_ind = 0; event_ind < SHOT_COUNT; event_ind++) {
            boardData.push_back({
                                    {"ch",    links[board % config.linkCount]->result[board / config.linkCount][event_ind]},
                                    {"ph_el", links[board % config.linkCount]->ph_el[board / config.linkCount][event_ind]},
                                    {"t",     (double)(links[board % config.linkCount]->times[event_ind] - links[board % config.linkCount]->times[0]) * 5e-6},
                                    {"t_raw", links[board % config.linkCount]->times[event_ind]}
            });
        }
        boardData[0]["t"] = 0;
        result["boards"].push_back(boardData);
        result["header"]["boards"].push_back(links[board % config.linkCount]->serials[board / config.linkCount]);
    }
    for(unsigned int link = 0; link < config.linkCount; link++){
        links[link]->disarm();
    }

    storage.saveDischarge(result);

    saving = false;
    saved = true;
    joinMe = true;
}
