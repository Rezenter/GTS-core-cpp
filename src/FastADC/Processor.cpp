//
// Created by ts_group on 27.09.2022.
//

#include "include/FastADC/Processor.h"

Processor::Processor(Config& config) : config(config){
    //init arrays
}

Processor::~Processor(){
    requestStop();
    associatedThread.join();
}

void Processor::process(size_t board){
    CAEN_DGTZ_X743_GROUP_t* group;
    CAEN_DGTZ_DecodeEvent(handles[board], encodedEvents[board][counts[board]], (void**)(&decodedEvents[board][counts[board]]));

    for(int groupIdx = 0; groupIdx < MAX_V1743_GROUP_SIZE; groupIdx++){
        if(eventDecoded->GrPresent[groupIdx]){
            group = &eventDecoded->DataGroup[groupIdx];

            //use fixed record length!
            size_t ch = 0;
            for(unsigned int cell = 0; cell < config->recordLength; cell++){
                channels[ch + 2 * groupIdx][cell] = config->offset + group->DataChannel[ch][cell] * 2500 / 4096;
            }
            ch = 1;
            for(unsigned int cell = 0; cell < config->recordLength; cell++){
                channels[ch + 2 * groupIdx][cell] = config->offset + group->DataChannel[ch][cell] * 2500 / 4096;
            }
        }
    }
    //results.push_back(entry);
}

bool Processor::payload() {
    //check, if all caens are ready and send data
    if(counts[0] < SHOT_COUNT) {
        mutex.lock();
        bool ready = eventFlags[0][counts[0]];
        mutex.unlock();
        if (ready) {
            process(0);
            counts[0]++;
        }
    }
    if(counts[1] < SHOT_COUNT) {
        mutex.lock();
        bool ready = eventFlags[1][counts[1]];
        mutex.unlock();
        if(ready){
            process(1);
            counts[1]++;
        }
    }
    if(counts[0] == SHOT_COUNT && counts[1] == SHOT_COUNT){
        return true;
    }
    return false;
}

void Processor::beforePayload() {
    counts.fill(0);
    for(size_t boardInd = 0; boardInd < 2; boardInd++){
        for(size_t evenInd = 0; evenInd < SHOT_COUNT; evenInd++){
            CAEN_DGTZ_AllocateEvent(handles[boardInd], (void**)(&decodedEvents[boardInd][evenInd]));
            eventFlags[boardInd][counts[boardInd]] = false;
        }
    }
}

void Processor::afterPayload() {
    //clear arrays
}

bool Processor::arm() {
    associatedThread = std::thread([&](){
        run();
    });
    return false;
}

bool Processor::disarm() {
    associatedThread.join();
    return false;
}
