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
        p++;
        CAEN_DGTZ_DecodeEvent(handle, encodedEvents[p], (void**)(&decodedEvents[p]));
        for(int groupIdx = 0; groupIdx < MAX_V1743_GROUP_SIZE; groupIdx++){
            if(decodedEvents[p]->GrPresent[groupIdx]){
                size_t ch1 = 2 * groupIdx;
                size_t ch2 = ch1 + 1;
                group = &decodedEvents[p]->DataGroup[groupIdx];
                if(groupIdx == 0){
                    times[p] = 5e-6 * group->TDC;
                }
                for(unsigned int cell = 0; cell < config.recordLength; cell++){
                    result[p][ch1][cell] = config.offset + group->DataChannel[0][cell] * resolution;
                    if(zeroInd[ch1].first < cell && cell <= zeroInd[ch1].second){
                        zero[p][ch1] += result[p][ch1][cell];
                    } else if(signalInd[ch1].first < cell && cell <= signalInd[ch1].second){
                        ph_el[p][ch1] += result[p][ch1][cell];
                    }

                    result[p][ch2][cell] = config.offset + group->DataChannel[1][cell] * resolution;
                    if(zeroInd[ch2].first < cell && cell <= zeroInd[ch2].second){
                        zero[p][ch2] += result[p][ch2][cell];
                    } else if(signalInd[ch2].first < cell && cell <= signalInd[ch2].second){
                        ph_el[p][ch2] += result[p][ch2][cell];
                    }
                }
                zero[p][ch1] *= 1 / (double)(zeroInd[ch1].second - zeroInd[ch1].first);
                zero[p][ch2] *= 1 / (double)(zeroInd[ch2].second - zeroInd[ch2].first);

                ph_el[p][ch1] -= zero[p][ch1] * (double)(signalInd[ch1].second - signalInd[ch1].first);
                ph_el[p][ch2] -= zero[p][ch2] * (double)(signalInd[ch2].second - signalInd[ch2].first);

                ph_el[p][ch1] *= 0.78125; // 0.3125e-9 * 1e-3 / (1e2 * 1.6e-19 * 1e4 * 2.5 * 2) * 2
                ph_el[p][ch2] *= 0.78125;
            }
        }
        processed++;
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
        times[evenInd] = 0.0;
        for(size_t ch_ind = 0; ch_ind < CH_COUNT; ch_ind++){
            zero[evenInd][ch_ind] = 0.0;
            ph_el[evenInd][ch_ind] = 0.0;
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
