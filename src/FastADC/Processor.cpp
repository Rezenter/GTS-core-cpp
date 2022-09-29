//
// Created by ts_group on 27.09.2022.
//

#include "include/FastADC/Processor.h"

Processor::Processor(Config& config) : config(config){

}

Processor::~Processor(){
    requestStop();
    associatedThread.join();
    for(auto & decodedEvent : decodedEvents) {
        CAEN_DGTZ_FreeEvent(handle, (void **) (&decodedEvent));
    }
}

bool Processor::payload() {
    //std::cout << processed.load() << ' ' << written.load() << std::endl;
    //std::cout << '!' << std::endl;
    int p = processed.load();
    int w = written.load(std::memory_order_acquire);
    if(p >= w){
        written.wait(w);
        w = written.load(std::memory_order_acquire);
    }
    while(p < w){
        /*
        CAEN_DGTZ_DecodeEvent(handles[board], encodedEvents[board][counts[board]], (void**)(&decodedEvents[board][counts[board]]));
        for(int groupIdx = 0; groupIdx < MAX_V1743_GROUP_SIZE; groupIdx++){
            if(decodedEvents[board][counts[board]]->GrPresent[groupIdx]){
                group = &decodedEvents[board][counts[board]]->DataGroup[groupIdx];

                for(unsigned int cell = 0; cell < config.recordLength; cell++){
                    result[board][counts[board]][2 * groupIdx][cell] = config.offset + group->DataChannel[0][cell] * resolution;
                    result[board][counts[board]][1 + 2 * groupIdx][cell] = config.offset + group->DataChannel[1][cell] * resolution;
                }
            }
        }
         */
        processed++;
        p++;
    }
    return false;
}

void Processor::beforePayload() {
    written.store(-1);
    processed.store(-1);
}

void Processor::afterPayload() {
    for(size_t evenInd = 0; evenInd < SHOT_COUNT; evenInd++) {
        CAEN_DGTZ_FreeEvent(handle, (void **) (&decodedEvents[evenInd]));
    }
    std::cout << "processor events: " << processed.load(std::memory_order_acquire) << std::endl;
}

bool Processor::arm() {
    for(size_t evenInd = 0; evenInd < SHOT_COUNT; evenInd++){
        CAEN_DGTZ_AllocateEvent(handle, (void**)(&decodedEvents[evenInd]));
        for(size_t ch_ind = 0; ch_ind < CH_COUNT; ch_ind++){
            for(size_t cell_ind = 0; cell_ind < PAGE_LENGTH; cell_ind++){
                result[evenInd][ch_ind][cell_ind] = 0.0;
            }
        }
    }
    associatedThread = std::thread([&](){
        run();
    });
    return false;
}

bool Processor::disarm() {
    requestStop();
    written.store(-1);
    written.notify_one();
    associatedThread.join();
    return false;
}
