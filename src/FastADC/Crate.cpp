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
        delete processors[count];
        delete processors[count * 2];
    }
}

bool Crate::init() {
    armed = false;
    for(unsigned int link = 0; link < config.linkCount; link++){
        std::cout << link << std::endl;
        processors[link] = new Processor(this->config, this->processed);
        processors[4 + link] = new Processor(this->config, this->processed);
        links[link] = new Link(config.links[link], processors[link], processors[4 + link]);
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
        processors[count]->arm();
        processors[4 + count]->arm();
        if(!links[count]->arm()) {
            return false;
        }
    }
    DAC1.fill(0);
    associatedThread = std::thread([&](){
        run();
    });
    for(size_t evenInd = 0; evenInd < SHOT_COUNT; evenInd++) {
        processed[evenInd] = new std::latch(config.caenCount);
    }
    std::cout << "armed" << std::endl;
    std::cout << "\n\n\nWARNING!!! not normalised on Elas!!!" << std::endl;
    return true;
}

Json Crate::disarm() {
    if(!armed){
        Json result = {
                {"err", "Crate is not armed!"}
        };
        return result;
    }
    armed = false;
    std::cout << "disarming..." << std::endl;
    Json result = {
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
    for(unsigned int link = 0; link < config.linkCount; link++){
        links[link]->disarm();
        links[link]->processors[0]->disarm();
        links[link]->processors[1]->disarm();
    }
    for(size_t board = 0; board < config.caenCount; board++){
        Json boardData = Json::array();
        for (size_t event_ind = 0; event_ind < SHOT_COUNT; event_ind++) {
            boardData.push_back({
                                    {"ch",    processors[board]->result[event_ind]},
                                    {"ph_el", processors[board]->ph_el[event_ind]},
                                    {"t",     processors[board]->times[event_ind]},
                                    {"DAC1",  DAC1[event_ind]}
                            });
        }

        result["boards"].push_back(boardData);
        result["header"]["boards"].push_back(links[board % 4]->serials[board / 4]);
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
    return true;
}

bool Crate::payload() {
    if(processed[currentEvent]->try_wait()){
        double ph_el = 0;
        for(size_t ch = 0; ch < 5; ch++){
            ph_el += processors[1]->ph_el[currentEvent][ch + 11];
        }
        ph_el = (currentEvent + 1) * 40.0 * 60;

        ph_el = fmax(0.0, ph_el) * 0.0585;
        ph_el = fmin(4095, ph_el);
        buffer.val = floor(ph_el);
        DAC1[currentEvent] = buffer.val;
        currentEvent++;
        sendto(sockfd, buffer.chars, 2,
               0, (const struct sockaddr *) &servaddr,
               sizeof(servaddr));
    }else{
        std::this_thread::sleep_for(std::chrono::microseconds(100));
        return false;
    }

    if(currentEvent == SHOT_COUNT){
        return true;
    }
    return false;
}

void Crate::beforePayload() {
    //open socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&servaddr, 0, sizeof(servaddr));
    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8080);
    servaddr.sin_addr.s_addr = inet_addr("192.168.10.56");
    currentEvent = 0;

    buffer.val = 0;
    sendto(sockfd, buffer.chars, 2,
           0, (const struct sockaddr *) &servaddr,
           sizeof(servaddr));
}

void Crate::afterPayload() {
    //close socket
    closesocket(sockfd);
    std::cout << "crate events: " << currentEvent << std::endl;
    buffer.val = 0;
    sendto(sockfd, buffer.chars, 2,
           0, (const struct sockaddr *) &servaddr,
           sizeof(servaddr));
    for(size_t evenInd = 0; evenInd < SHOT_COUNT; evenInd++) {
        delete processed[evenInd];
    }
}
