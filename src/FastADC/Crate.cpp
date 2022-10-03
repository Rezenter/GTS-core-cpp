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
    for(unsigned int count = 0; count < config.caenCount; count++){
        delete caens[count];
        delete processors[count];
    }
}

bool Crate::init() {
    armed = false;
    for(unsigned int count = 0; count < config.caenCount; count++){
        processors[count] = new Processor(this->config);
        caens[count] = new CAEN743(config.links[count], config.nodes[count], processors[count]);
        caens[count]->init(config);
        if(!caens[count]->isAlive()){
            std::cout << "board " << count << " not initialised!" << std::endl;
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
    for(unsigned int count = 0; count < config.caenCount; count++){
        processors[count]->arm();
        if(!caens[count]->arm()) {
            return false;
        }
    }
    DAC1.fill(0);
    associatedThread = std::thread([&](){
        run();
    });
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
    for(unsigned int count = 0; count < config.caenCount; count++){
        caens[count]->disarm();
        processors[count]->disarm();
        Json board = Json::array();
        for(size_t event_ind = 0; event_ind < SHOT_COUNT; event_ind++){
            board.push_back({
                                    {"ch", processors[count]->result[event_ind]},
                                    {"ph_el", processors[count]->ph_el[event_ind]},
                                    {"t", processors[count]->times[event_ind]},
                                    {"DAC1", DAC1[event_ind]}
                            });
        }

        result["boards"].push_back(board);
        result["header"]["boards"].push_back(caens[count]->getSerial());
        caens[count]->releaseMemory();
    }
    this->requestStop();
    std::cout << "all joined" << std::endl;
    associatedThread.join();
    return result;
}

bool Crate::isAlive() {
    for(unsigned int count = 0; count < config.caenCount; count++){
        if(!caens[count]->isAlive()){
            std::cout << "board " << count << " dead!" << std::endl;
            return false;
        }
    }
    return true;
}

bool Crate::payload() {
    for(unsigned int count = 0; count < config.caenCount; count++){
        if(processors[count]->processed.load(std::memory_order_acquire) < eventCount){
            return false;
        }
    }
    double ph_el = 0;
    for(size_t ch = 0; ch < 5; ch++){
        ph_el += processors[1]->ph_el[eventCount][ch + 11];
    }
    ph_el = fmax(0.0, ph_el) * 0.0585;
    ph_el = fmin(4095, ph_el);
    buffer.val = floor(ph_el);
    DAC1[eventCount] = buffer.val;
    eventCount++;
    sendto(sockfd, buffer.chars, 2,
           0, (const struct sockaddr *) &servaddr,
           sizeof(servaddr));
    return false;
}

void Crate::beforePayload() {
    //open socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&servaddr, 0, sizeof(servaddr));
    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8080);
    servaddr.sin_addr.s_addr = inet_addr("192.168.10.49");
    eventCount = 0;

    buffer.val = 0;
    sendto(sockfd, buffer.chars, 2,
           0, (const struct sockaddr *) &servaddr,
           sizeof(servaddr));
}

void Crate::afterPayload() {
    //close socket
    closesocket(sockfd);
    std::cout << "crate events: " << eventCount << std::endl;
    eventCount = 0;
    buffer.val = 0;
    sendto(sockfd, buffer.chars, 2,
           0, (const struct sockaddr *) &servaddr,
           sizeof(servaddr));
}
