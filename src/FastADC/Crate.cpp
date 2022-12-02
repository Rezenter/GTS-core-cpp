//
// Created by ts_group on 6/28/2020.
//

#include "include/FastADC/Crate.h"

//debug
#include <iostream>
//debug


Crate::Crate(Config &config) : config(config) {
    init();
}

Crate::~Crate(){
    disarm();
    for(unsigned int count = 0; count < config.linkCount; count++){
        delete links[count];
    }
}

bool Crate::init() {
    armed = false;
    for(unsigned int link = 0; link < config.linkCount; link++){
        std::cout << link << std::endl;
        links[link] = new Link(config.links[link], processed);
        links[link]->init(config);
        if(!links[link]->isAlive()){
            std::cout << "board " << link << " not initialised!" << std::endl;
            return false;
        }
    }
    return true;
}

bool Crate::arm() {
    if(armed){
        return false;
    }
    armed = true;
    for(unsigned int count = 0; count < config.linkCount; count++){
        if(!links[count]->arm()) {
            return false;
        }
    }
    DAC1.fill(0);
    associatedThread = std::thread([&](){
        run();
    });
    for(size_t evenInd = 0; evenInd < SHOT_COUNT; evenInd++) {
        //processed[evenInd] = new std::latch(config.caenCount);
        processed[evenInd] = new std::latch(config.linkCount);
    }
    std::cout << "armed" << std::endl;
    std::cout << "\n\n\nWARNING!!! not normalised on Elas!!!" << std::endl;
    return true;
}

Json Crate::disarm() {
    if(!armed){
        result = {
                {"err", "Crate is not armed!"}
        };
        return result;
    }
    armed = false;
    std::cout << "disarming..." << std::endl;
    for(unsigned int link = 0; link < config.linkCount; link++){
        links[link]->disarm();
    }

    this->requestStop();
    std::cout << "all joined" << std::endl;
    associatedThread.join();
    return result;
}

bool Crate::isAlive() {
    for(unsigned int count = 0; count < config.linkCount; count++){
        if(!links[count]->isAlive()){
            std::cout << "board " << count << " dead!" << std::endl;
            return false;
        }
    }
    std::this_thread::sleep_for(std::chrono::seconds(5));
    return true;
}

bool Crate::payload() {
    if(processed[currentEvent]->try_wait()){
        double ph_el = 0;
        for(size_t ch = 0; ch < 5; ch++){
            ph_el += links[1]->ph_el[0][currentEvent][ch + 11];
        }
        //ph_el = (currentEvent + 1) * 40.0 * 60;
        ph_el = fmax(0.0, ph_el) * 0.023; // FOR 1.6 J ONLY!!!
        ph_el = fmin(4095, ph_el);
        buffer.val[0] = floor(ph_el);
        DAC1[currentEvent] = buffer.val[0];

        buffer.val[1] = fmin(3276, 0); // limit gas puff to 4 volts

        currentEvent++;
        sendto(sockfd, buffer.chars, 4,
               0, (const struct sockaddr *) &servaddr,
               sizeof(servaddr));
        return currentEvent == SHOT_COUNT;
    }else{
        std::this_thread::sleep_for(std::chrono::microseconds(100));
        return false;
    }
}

void Crate::beforePayload() {
    result = {};
    unsigned long long mask = 1 << 2;
    SetThreadAffinityMask(GetCurrentThread(), mask);
    std::cout << "Crate thread: " << SetThreadAffinityMask(GetCurrentThread(), mask) << std::endl;
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

    result = {
            {"header", {
                               {"version", 3},
                               {"eventLength", config.recordLength},
                               {"frequency", config.freqStr()},
                               {"boards", Json::array()},
                               {"offset", config.offset},
                               {"aux", config.aux_args}
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
                                    {"DAC1",  DAC1[event_ind]},
                                    {"t_raw", links[board % config.linkCount]->times[event_ind]}
            });
        }
        boardData[0]["t"] = 0;
        result["boards"].push_back(boardData);
        result["header"]["boards"].push_back(links[board % config.linkCount]->serials[board / config.linkCount]);
    }
}
