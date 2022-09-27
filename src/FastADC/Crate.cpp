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
    for(unsigned int count = 0; count < config.processCount; count++){
        delete processors[count];
    }
    for(unsigned int count = 0; count < config.caenCount; count++){
        delete caens[count];
    }
}

bool Crate::init() {
    armed = false;
    for(unsigned int count = 0; count < config.processCount; count++){
        processors[count] = new Processor(this->config);
    }
    for(unsigned int count = 0; count < config.caenCount; count++){
        caens[count] = new CAEN743(config.links[count], config.nodes[count], processors[config.links[count]]);
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
        if(!caens[count]->arm()) {
            return false;
        }
    }
    associatedThread = std::thread([&](){
        run();
    });
    std::cout << "armed" << std::endl;
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
    for(unsigned int count = 0; count < config.caenCount; count++){
        caens[count]->disarm();
    }
    this->requestStop();
    Json result = {
        {"header", {
                {"version", 2},
                {"error", true},
                {"eventLength", config.recordLength},
                {"frequency", config.freqStr()},
                {"boards", Json::array()},
                {"triggerThreshold", config.triggerThreshold},
                {"offset", config.offset},
                {"aux", config.aux_args}
            }
        },
        {"boards", Json::array()}
    };

    for(unsigned int count = 0; count < config.caenCount; count++) {
        result["boards"].push_back(caens[count]->waitTillProcessed());
        result["header"]["boards"].push_back(caens[count]->getSerial());
        caens[count]->releaseMemory();
    }
    std::cout << "all joined" << std::endl;
    result["header"]["error"] = false;
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
    //check, if all caens are ready and send data
    std::cout << "WRONG!!!" << std::endl;
    for(unsigned int index = 0; index < config.caenCount; index++){
        if(!caens[index]->eventReady){
            return false;
        }
    }
    for(unsigned int index = 0; index < config.caenCount; index++){
        caens[index]->eventReady = false;
    }
    buffer.val = eventCount * 4096 * 10 / 100;
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
}

void Crate::afterPayload() {
    //close socket
    closesocket(sockfd);
    eventCount = 0;
}
