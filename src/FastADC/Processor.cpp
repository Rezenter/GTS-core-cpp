//
// Created by ts_group on 27.09.2022.
//

#include "include/FastADC/Processor.h"

Processor::Processor(Config& config, std::array<std::latch*, SHOT_COUNT>& processed) : config(config), processed(processed){

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

    if(written->try_acquire_for(std::chrono::microseconds(100))){
        for(int groupIdx = 0; groupIdx < MAX_V1743_GROUP_SIZE; groupIdx++){
            if(decodedEvents[current_index]->GrPresent[groupIdx]){
                size_t ch1 = 2 * groupIdx;
                size_t ch2 = ch1 + 1;
                group = &decodedEvents[current_index]->DataGroup[groupIdx];
                if(groupIdx == 0){
                    times[current_index] = 5e-6 * (double)group->TDC;
                }
                for(unsigned int cell = 0; cell < config.recordLength; cell++){
                    result[current_index][ch1][cell] = config.offset + group->DataChannel[0][cell] * resolution;
                    if(zeroInd[ch1].first < cell && cell <= zeroInd[ch1].second){
                        zero[current_index][ch1] += result[current_index][ch1][cell];
                    } else if(signalInd[ch1].first < cell && cell <= signalInd[ch1].second){
                        ph_el[current_index][ch1] += result[current_index][ch1][cell];
                    }

                    result[current_index][ch2][cell] = config.offset + group->DataChannel[1][cell] * resolution;
                    if(zeroInd[ch2].first < cell && cell <= zeroInd[ch2].second){
                        zero[current_index][ch2] += result[current_index][ch2][cell];
                    } else if(signalInd[ch2].first < cell && cell <= signalInd[ch2].second){
                        ph_el[current_index][ch2] += result[current_index][ch2][cell];
                    }
                }
                zero[current_index][ch1] *= 1 / (double)(zeroInd[ch1].second - zeroInd[ch1].first);
                zero[current_index][ch2] *= 1 / (double)(zeroInd[ch2].second - zeroInd[ch2].first);

                ph_el[current_index][ch1] -= zero[current_index][ch1] * (double)(signalInd[ch1].second - signalInd[ch1].first);
                ph_el[current_index][ch2] -= zero[current_index][ch2] * (double)(signalInd[ch2].second - signalInd[ch2].first);

                ph_el[current_index][ch1] *= 0.78125; // 0.3125e-9 * 1e-3 / (1e2 * 1.6e-19 * 1e4 * 2.5 * 2) * 2
                ph_el[current_index][ch2] *= 0.78125;
            }
        }
        processed[current_index]->count_down();
        current_index++;
    }
    return false;
}

void Processor::beforePayload() {
    current_index = 0;
    written = new std::counting_semaphore<SHOT_COUNT>(0);
}

void Processor::afterPayload() {
    for(size_t evenInd = 0; evenInd < SHOT_COUNT; evenInd++) {
        CAEN_DGTZ_FreeEvent(handle, (void **) (&decodedEvents[evenInd]));
    }
    std::cout << "processor events: " << current_index << std::endl;
    delete written;
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
    associatedThread.join();
    return false;
}
