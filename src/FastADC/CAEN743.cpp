//
// Created by user on 6/18/2020.
//

//debug
#include <iostream>
//debug

#include "include/FastADC/CAEN743.h"

int CAEN743::init(Config& config){
    this->config = &config;
    ret = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_OpticalLink, address, chain_node, 0, &handle);
    processor->handle = handle;
    if(ret != CAEN_DGTZ_Success) {
        printf("Can't open digitizer (%d, %d)\n", address, chain_node);
        return CAEN_Error_Connection;
    }

    printf("Connected to CAEN with address %d, %d\n", address, chain_node);

    ret = CAEN_DGTZ_GetInfo(handle, &boardInfo);

    ret = CAEN_DGTZ_SetSAMSamplingFrequency(handle, config.frequency);
    ret = CAEN_DGTZ_SetRecordLength(handle,config.recordLength);
    for(int sam_idx = 0; sam_idx < MAX_V1743_GROUP_SIZE; sam_idx++){
        ret = CAEN_DGTZ_SetSAMPostTriggerSize(handle, sam_idx, config.triggerDelay);
    }

    ret = CAEN_DGTZ_SetGroupEnableMask(handle,0b11111111);
    ret = CAEN_DGTZ_SetChannelSelfTrigger(handle,CAEN_DGTZ_TRGMODE_DISABLED,0b11111111);
    //ret = CAEN_DGTZ_SetSWTriggerMode(handle,CAEN_DGTZ_TRGMODE_DISABLED);
    ret = CAEN_DGTZ_SetSWTriggerMode(handle,CAEN_DGTZ_TRGMODE_EXTOUT_ONLY);
    ret = CAEN_DGTZ_SetMaxNumEventsBLT(handle, MAX_TRANSFER);

    if(chain_node == 0){
        ret = CAEN_DGTZ_SetIOLevel(handle, CAEN_DGTZ_IOLevel_NIM);
    }else{
        ret = CAEN_DGTZ_SetIOLevel(handle, CAEN_DGTZ_IOLevel_TTL);
    }

    ret = CAEN_DGTZ_SetAcquisitionMode(handle,CAEN_DGTZ_SW_CONTROLLED);
    ret = CAEN_DGTZ_SetExtTriggerInputMode(handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY);

    for(int ch = 0; ch < 16; ch++){
        ret = CAEN_DGTZ_SetChannelDCOffset(handle, ch, config.offsetADC);
    }
    //ret = CAEN_DGTZ_SetChannelTriggerThreshold(handle, 0, config.triggerThresholdADC);


    if(ret != CAEN_DGTZ_Success) {
        std::cout << "ADC " << (int)address << " initialisation error " << ret << std::endl;
        return 4;
    }

    uint32_t size;
    if(ret != CAEN_DGTZ_Success){
        std::cout << "ADC " << (int)address << " allocation error " << ret << std::endl;
        return 6;
    }
    for(size_t event_ind = 0; event_ind < SHOT_COUNT; event_ind++){
        ret = CAEN_DGTZ_MallocReadoutBuffer(handle, &processor->readoutBuffer[event_ind], &size);
        if(ret != CAEN_DGTZ_Success){
            std::cout << "ADC " << (int)address << " allocation error " << ret << std::endl;
            return 6;
        }
    }

    if(ret == CAEN_DGTZ_Success) {
        initialized = true;
        return CAEN_Success;
    }
    std::cout << "wtf?" << std::endl;
    return 8;
}

bool CAEN743::isAlive(){
    return initialized && CAEN_DGTZ_GetInfo(handle, &boardInfo) == CAEN_DGTZ_Success;
}

CAEN743::~CAEN743() {
    disarm();
    if(associatedThread.joinable()){
        associatedThread.join();
    }
    //std::cout << "caen destructor...";
    if(initialized){
        for(size_t event_ind = 0; event_ind < SHOT_COUNT; event_ind++){
            CAEN_DGTZ_FreeReadoutBuffer(&processor->readoutBuffer[event_ind]);
        }
    }

    if(handle){
        CAEN_DGTZ_CloseDigitizer(handle);
    }
    //std::cout << "OK" << std::endl;
}

bool CAEN743::arm() {
    ret = CAEN_DGTZ_ClearData(handle);
    ret = CAEN_DGTZ_SWStartAcquisition(handle);

    associatedThread = std::thread([&](){
        run();
    });
    return true;
}

bool CAEN743::disarm() {
    requestStop();
    associatedThread.join();
    return true;
}

bool CAEN743::payload() {
    CAEN_DGTZ_ReadData(handle,CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, processor->readoutBuffer[currentEvent], &processor->readoutBufferSize[currentEvent]);
    numEvents = floor(static_cast<double>(processor->readoutBufferSize[currentEvent]) / 34832);
    if(numEvents != 0){
        processor->written->release();
        currentEvent+=numEvents;
        if(currentEvent >= SHOT_COUNT){
            std::cout << "CAEN 101+" << std::endl;
            return true;
        }
    }
    return false;
}

void CAEN743::afterPayload() {
    ret = CAEN_DGTZ_SWStopAcquisition(handle);
    if(ret != 0){
        std::cout << "failed to stop ADC = " << ret << std::endl;
    }
    ret = CAEN_DGTZ_ClearData(handle);
}

void CAEN743::beforePayload() {
    currentEvent = 0;
}

