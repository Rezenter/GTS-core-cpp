//
// Created by user on 6/18/2020.
//

//debug
#include <iostream>
//debug

#include "CAEN743.h"

unsigned char CAEN743::caenCount = 0;

int CAEN743::init(Config& config){
    this->config = &config;
    ret = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_OpticalLink, address, chain_node, 0, &handle);
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
    ret = CAEN_DGTZ_SetSWTriggerMode(handle,CAEN_DGTZ_TRGMODE_DISABLED);
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
    ret = CAEN_DGTZ_MallocReadoutBuffer(handle, &buffer, &size);
    if(ret != CAEN_DGTZ_Success){
        std::cout << "ADC " << (int)address << " allocation error " << ret << std::endl;
        return 6;
    }
    bufferSize = 0;

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
    if(associatedThread.joinable()){
        associatedThread.join();
    }
    //std::cout << "caen destructor...";
    if(initialized){
        CAEN_DGTZ_FreeReadoutBuffer(&buffer); // some shit
    }

    if(handle){
        CAEN_DGTZ_CloseDigitizer(handle);
    }
    //std::cout << "OK" << std::endl;
}

void CAEN743::process() {
    results.clear();
    CAEN_DGTZ_X743_EVENT_t* eventDecoded = nullptr;
    ret = CAEN_DGTZ_AllocateEvent(handle, (void**)(&eventDecoded));

    std::array<std::array<double, 1024>, 16> channels{};

    CAEN_DGTZ_X743_GROUP_t* group;
    for(char* event : events){
        Json entry = {};
        CAEN_DGTZ_DecodeEvent(handle, event, (void **) &eventDecoded);


        for(int groupIdx = 0; groupIdx < MAX_V1743_GROUP_SIZE; groupIdx++){
            if(eventDecoded->GrPresent[groupIdx]){
                group = &eventDecoded->DataGroup[groupIdx];
                if(!entry.contains("t")){
                    entry["t"] = 1000.0 * group->TDC / CLOCK_FREQ;
                }

                for(int ch = 0; ch < 2; ch ++){
                    for(int cell = 0; cell < config->recordLength; cell++){
                        channels[ch + 2 * groupIdx][cell] = config->offset + group->DataChannel[ch][cell] * 2500 / 4096;
                    }
                }
            }else{
                for(int ch = 0; ch < 2; ch ++){
                    for(int cell = 0; cell < config->recordLength; cell++){
                        channels[ch + 2 * groupIdx][cell] = -12.34;
                    }
                }
            }

        }
        entry["ch"] = channels;
        results.push_back(entry);
        delete[] event;
    }
    events.clear();
    ret = CAEN_DGTZ_FreeEvent(handle, (void**)&eventDecoded);
}

bool CAEN743::arm() {
    ret = CAEN_DGTZ_ClearData(handle);
    ret = CAEN_DGTZ_SWStartAcquisition(handle);
    return true;

}

bool CAEN743::disarm() {
    requestStop();
    return true;
}

bool CAEN743::payload() {
    CAEN_DGTZ_ReadData(handle,CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer, &bufferSize);
    ret = CAEN_DGTZ_GetNumEvents(handle, buffer, bufferSize, &numEvents);
    if(numEvents != 0){
        for(counter = 0; counter < numEvents; counter++){
            CAEN_DGTZ_GetEventInfo(handle, buffer, bufferSize, counter, &eventInfo, &eventEncoded);
            char* singleEvent = new char[EVT_SIZE];
            memcpy(singleEvent, eventEncoded, EVT_SIZE);
            events.push_back(singleEvent);
        }
    }
    return false;
}

void CAEN743::afterPayload() {
    //std::cout << "after payload" << std::endl;
    ret = CAEN_DGTZ_SWStopAcquisition(handle);
    if(ret != 0){
        std::cout << "failed to stop ADC = " << ret << std::endl;
    }
    ret = CAEN_DGTZ_ClearData(handle);
    //std::cout << "stopped adc " << (int)address << std::endl << std::flush;
    process();
    //std::cout << "after payload finished" << std::endl;
}

void CAEN743::beforePayload() {
    //ret = CAEN_DGTZ_ClearData(handle);
}


Json CAEN743::waitTillProcessed() {
    associatedThread.join();
    //std::cout << "thread " << (int)address << " joined" << std::endl;
    return results;
}

bool CAEN743::cyclicReadout() {
    associatedThread = std::thread([&](){
        run();
    });
    return false;
}

bool CAEN743::releaseMemory() {
    results.clear();
    for(char* event : events){
        delete[] event;
    }
    events.clear();
    return false;
}

