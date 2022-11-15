//
// Created by user on 6/18/2020.
//

//debug
#include <iostream>
//debug

#include "include/FastADC/Link.h"

int Link::init(Config& config){
    this->config = &config;

    for(int node = 0; node < 2; node++) {
        //ret = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_OpticalLink, link, chain_node, 0, &handle);
        ret = CAEN_DGTZ_OpenDigitizer2(CAEN_DGTZ_OpticalLink, reinterpret_cast<void *>(&link), node, 0, &handles[node]);
        processors[node]->handle = handles[node];
        if (ret != CAEN_DGTZ_Success) {
            printf("Can't open digitizer (%d, %d)\n", link, node);
            return 1;
        }
        printf("Connected to CAEN with link %d, %d\n", link, node);

        ret = CAEN_DGTZ_GetInfo(handles[node], &boardInfo);
        serials[node] = boardInfo.SerialNumber;

        ret = CAEN_DGTZ_SetSAMSamplingFrequency(handles[node], config.frequency);
        ret = CAEN_DGTZ_SetRecordLength(handles[node], config.recordLength);
        for (int sam_idx = 0; sam_idx < MAX_V1743_GROUP_SIZE; sam_idx++) {
            ret = CAEN_DGTZ_SetSAMPostTriggerSize(handles[node], sam_idx, config.triggerDelay);
        }

        ret = CAEN_DGTZ_SetGroupEnableMask(handles[node], 0b11111111);
        ret = CAEN_DGTZ_SetChannelSelfTrigger(handles[node], CAEN_DGTZ_TRGMODE_DISABLED, 0b11111111);
        //ret = CAEN_DGTZ_SetSWTriggerMode(handles[node],CAEN_DGTZ_TRGMODE_DISABLED);
        ret = CAEN_DGTZ_SetSWTriggerMode(handles[node], CAEN_DGTZ_TRGMODE_EXTOUT_ONLY);
        ret = CAEN_DGTZ_SetMaxNumEventsBLT(handles[node], MAX_TRANSFER);

        if (node == 0) {
            ret = CAEN_DGTZ_SetIOLevel(handles[node], CAEN_DGTZ_IOLevel_NIM);
        } else {
            ret = CAEN_DGTZ_SetIOLevel(handles[node], CAEN_DGTZ_IOLevel_TTL);
        }

        ret = CAEN_DGTZ_SetAcquisitionMode(handles[node], CAEN_DGTZ_SW_CONTROLLED);
        ret = CAEN_DGTZ_SetExtTriggerInputMode(handles[node], CAEN_DGTZ_TRGMODE_ACQ_ONLY);

        for (int ch = 0; ch < 16; ch++) {
            ret = CAEN_DGTZ_SetChannelDCOffset(handles[node], ch, config.offsetADC);
        }
        //ret = CAEN_DGTZ_SetChannelTriggerThreshold(handles[node], 0, config.triggerThresholdADC);

        if (ret != CAEN_DGTZ_Success) {
            std::cout << "ADC " << link << " initialisation error " << ret << std::endl;
            return 4;
        }

        uint32_t size;
        for (size_t event_ind = 0; event_ind < SHOT_COUNT; event_ind++) {
            ret = CAEN_DGTZ_MallocReadoutBuffer(handles[node], &processors[node]->readoutBuffer[event_ind], &size);
            if (ret != CAEN_DGTZ_Success) {
                std::cout << "ADC " << link << " allocation error " << ret << std::endl;
                return 6;
            }
        }
    }
    if(ret == CAEN_DGTZ_Success) {
        initialized = true;
        return 0;
    }
    std::cout << "wtf?" << std::endl;
    return 8;
}

bool Link::isAlive(){
    bool alive = initialized;
    for(int node = 0; node < 2; node++) {
        alive &= CAEN_DGTZ_GetInfo(handles[node], &boardInfo) == CAEN_DGTZ_Success;
    }
    return alive;
}

Link::~Link() {
    disarm();
    if(associatedThread.joinable()){
        associatedThread.join();
    }
    //std::cout << "caen destructor...";
    if(initialized){
        for(int node = 0; node < 2; node++) {
            for (size_t event_ind = 0; event_ind < SHOT_COUNT; event_ind++) {
                CAEN_DGTZ_FreeReadoutBuffer(&processors[node]->readoutBuffer[event_ind]);
            }
            CAEN_DGTZ_CloseDigitizer(handles[node]);
        }
    }

    //std::cout << "OK" << std::endl;
}

bool Link::arm() {
    for(int node = 0; node < 2; node++) {
        CAEN_DGTZ_ClearData(handles[node]);
        CAEN_DGTZ_SWStartAcquisition(handles[node]);
    }

    associatedThread = std::thread([&](){
        run();
    });
    return true;
}

bool Link::disarm() {
    requestStop();
    associatedThread.join();
    return true;
}

bool Link::payload() {
    CAEN_DGTZ_ReadData(handles[0],CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, processors[0]->readoutBuffer[currentEvent[0]], &processors[0]->readoutBufferSize[currentEvent[0]]);
    numEvents = floor(static_cast<double>(processors[0]->readoutBufferSize[currentEvent[0]]) / 34832);
    if(numEvents != 0){
        processors[0]->written->release();
        currentEvent[0]+=numEvents;

        CAEN_DGTZ_ReadData(handles[1],CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, processors[1]->readoutBuffer[currentEvent[1]], &processors[1]->readoutBufferSize[currentEvent[1]]);
        currentEvent[1]+= floor(static_cast<double>(processors[1]->readoutBufferSize[currentEvent[1]]) / 34832);
        processors[1]->written->release();

        if(currentEvent[0] >= SHOT_COUNT){
            std::cout << "CAEN 101+" << std::endl;
            return true;
        }
    }
    return false;
}

void Link::afterPayload() {
    for(int node = 0; node < 2; node++) {
        ret = CAEN_DGTZ_SWStopAcquisition(handles[node]);
        if (ret != 0) {
            std::cout << "failed to stop ADC = " << ret << std::endl;
        }
        ret = CAEN_DGTZ_ClearData(handles[node]);
    }
}

void Link::beforePayload() {
    currentEvent[0] = 0;
    currentEvent[1] = 0;
}

