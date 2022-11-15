//
// Created by ts_group on 27.09.2022.
//

#include "include/FastADC/Processor.h"

Processor::Processor(Config& config, std::array<std::latch*, SHOT_COUNT>& processed) : config(config), processed(processed){

}

Processor::~Processor(){
    requestStop();
    associatedThread.join();
}

bool Processor::payload() {
    if(written->try_acquire_for(std::chrono::microseconds(100))){
        int pack_size = floor(static_cast<double>(readoutBufferSize[current_index]) / 34832);
        char* group_pointer = readoutBuffer[current_index];
        for(int event_subindex = 0; event_subindex < pack_size; event_subindex++) {
            group_pointer += 16; // skip header
            timestampConverter.integer = 0;
            timestampConverter.bytes[0] = *(group_pointer + 4 * 14 + 3);
            timestampConverter.bytes[1] = *(group_pointer + 4 * 15 + 3);
            timestampConverter.bytes[2] = *(group_pointer + 4 * 16 + 3);
            timestampConverter.bytes[3] = *(group_pointer + 4 * 17 + 3);
            timestampConverter.bytes[4] = *(group_pointer + 4 * 18 + 3);
            times[current_index] = 5e-6 * timestampConverter.integer;

            for (int groupIdx = 0; groupIdx < MAX_V1743_GROUP_SIZE; groupIdx++) {
                size_t ch1 = 2 * groupIdx;
                size_t ch2 = ch1 + 1;

                for(unsigned short sector = 0; sector < 64; sector++){ // 64 sectors per 1024 cell page
                    group_pointer += 4; // skip trash line, not mentioned in datasheet
                    for (unsigned int cell = 0; cell < 16; cell++) {
                        currentCell = sector * 16 + cell;
                        result[current_index][ch1][currentCell] = *reinterpret_cast<unsigned short*>((group_pointer + 4 * cell)) & 0x0FFF;

                        //what disarms processor before button???

                        if (zeroInd[ch1].first < currentCell && currentCell <= zeroInd[ch1].second) {
                            zero[current_index][ch1] += result[current_index][ch1][currentCell];
                        } else if (signalInd[ch1].first < currentCell && currentCell <= signalInd[ch1].second) {
                            ph_el[current_index][ch1] += result[current_index][ch1][currentCell];
                        }


                        result[current_index][ch2][sector * 16 + cell] = *reinterpret_cast<unsigned short*>((group_pointer + 4 * cell + 1)) >> 4;

                        if (zeroInd[ch2].first < currentCell && currentCell <= zeroInd[ch2].second) {
                            zero[current_index][ch2] += result[current_index][ch2][currentCell];
                        } else if (signalInd[ch2].first < currentCell && currentCell <= signalInd[ch2].second) {
                            ph_el[current_index][ch2] += result[current_index][ch2][currentCell];
                        }
                    }
                    group_pointer += 4 * 16;
                }
                zero[current_index][ch1] /= zeroInd[ch1].second - zeroInd[ch1].first;
                zero[current_index][ch2] /= zeroInd[ch2].second - zeroInd[ch2].first;

                ph_el[current_index][ch1] -=
                        zero[current_index][ch1] * (signalInd[ch1].second - signalInd[ch1].first);
                ph_el[current_index][ch2] -=
                        zero[current_index][ch2] * (signalInd[ch2].second - signalInd[ch2].first);

                //ph_el[current_index][ch1] *= 0.78125; // 0.3125e-9 * 1e-3 / (1e2 * 1.6e-19 * 1e4 * 2.5 * 2) * 2
                //ph_el[current_index][ch2] *= 0.78125;
            }

            processed[current_index]->count_down();
            current_index++;
            if(current_index >= SHOT_COUNT){
                //std::cout << "processed 101" << std::endl;
                return true;
            }
        }
    }
    return false;
}

void Processor::beforePayload() {
    current_index = 0;
    written = new std::counting_semaphore<SHOT_COUNT>(0);
}

void Processor::afterPayload() {
    std::cout << "processor events: " << current_index << std::endl;
    delete written;
}

bool Processor::arm() {
    for(size_t evenInd = 0; evenInd < SHOT_COUNT; evenInd++){
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
