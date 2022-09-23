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

bool Crate::init() {
    for(int count = 0; count < config.caenCount; count++){
        caens[count] = new CAEN743(config.links[count], config.nodes[count]);
        caens[count]->init(config);
        if(!caens[count]->isAlive()){
            std::cout << "board " << count << " not initialised!" << std::endl;
            return false;
        }
    }
    return true;
}

bool Crate::arm() {
    for(int count = 0; count < config.caenCount; count++){
        if(!caens[count]->arm()) {
            return false;
        }
    }
    for(int count = 0; count < config.caenCount; count++){
        caens[count]->cyclicReadout();
    }
    associatedThread = std::thread([&](){
        run();
    });
    std::cout << "armed" << std::endl;
    return true;
}

Json Crate::disarm() {
    std::cout << "disarming..." << std::endl;
    for(int count = 0; count < config.caenCount; count++){
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

    for(int count = 0; count < config.caenCount; count++) {
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
    for(int count = 0; count < config.caenCount; count++){
        if(!caens[count]->isAlive()){
            std::cout << "board " << count << " dead!" << std::endl;
            return false;
        }
    }
    return true;
}

bool Crate::payload() {
    //check, if all caens are ready and send data
    for(int index = 0; index < config.caenCount; index++){
        if(!caens[index]->eventReady){
            return false;
        }
    }
    for(int index = 0; index < config.caenCount; index++){
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
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){
        perror("socket creation failed");
    }

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
