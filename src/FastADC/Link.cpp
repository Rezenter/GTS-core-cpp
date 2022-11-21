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
        ret = CAEN_DGTZ_OpenDigitizer2(CAEN_DGTZ_OpticalLink, reinterpret_cast<void *>(&link), node, 0, &handles[node]);
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

        ret = CAEN_DGTZ_MallocReadoutBuffer(handles[node], &readoutBuffer, &size);
        if (ret != CAEN_DGTZ_Success) {
            std::cout << "ADC " << link << " allocation error " << ret << std::endl;
            return 6;
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
            CAEN_DGTZ_FreeReadoutBuffer(&readoutBuffer);
            CAEN_DGTZ_CloseDigitizer(handles[node]);
        }
    }

    //std::cout << "OK" << std::endl;
}

bool Link::arm() {
    for(size_t evenInd = 0; evenInd < SHOT_COUNT; evenInd++){
        times[evenInd] = 0.0;
        for(size_t node = 0; node < 2; node++){
            for(size_t ch_ind = 0; ch_ind < CH_COUNT; ch_ind++){
                zero[node][evenInd][ch_ind] = 0.0;
                ph_el[node][evenInd][ch_ind] = 0.0;
                for(size_t cell_ind = 0; cell_ind < PAGE_LENGTH; cell_ind++){
                    result[node][evenInd][ch_ind][cell_ind] = 0.0;

                }
            }
        }
    }
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
    CAEN_DGTZ_ReadData(handles[0],CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, readoutBuffer, &readoutBufferSize);
    if(readoutBufferSize == EVT_SIZE || preload){
        char* group_pointer = readoutBuffer + 16;
        timestampConverter.integer = 0;
        timestampConverter.bytes[0] = *(group_pointer + 4 * 14 + 3);
        timestampConverter.bytes[1] = *(group_pointer + 4 * 15 + 3);
        timestampConverter.bytes[2] = *(group_pointer + 4 * 16 + 3);
        timestampConverter.bytes[3] = *(group_pointer + 4 * 17 + 3);
        timestampConverter.bytes[4] = *(group_pointer + 4 * 18 + 3);
        times[currentEvent] = timestampConverter.integer;

        for (int groupIdx = 0; groupIdx < MAX_V1743_GROUP_SIZE; groupIdx++) {
            size_t ch1 = 2 * groupIdx;
            size_t ch2 = ch1 + 1;

            for (unsigned short sector = 0; sector < 64; sector++) { // 64 sectors per 1024 cell page
                group_pointer += 4; // skip trash line, not mentioned in datasheet
                for (unsigned int cell = 0; cell < 16; cell++) {
                    currentCell = sector * 16 + cell;
                    result[0][currentEvent][ch1][currentCell] =
                            *reinterpret_cast<unsigned short *>((group_pointer + 4 * cell)) & 0x0FFF;
                    result[0][currentEvent][ch1][currentCell] = (result[0][currentEvent][ch1][currentCell] & 0b011111111111) | (~result[0][currentEvent][ch1][currentCell] & 0b100000000000);
                    if (zeroInd[ch1].first < currentCell && currentCell <= zeroInd[ch1].second) {
                        zero[0][currentEvent][ch1] += result[0][currentEvent][ch1][currentCell];
                    } else if (signalInd[ch1].first < currentCell && currentCell <= signalInd[ch1].second) {
                        ph_el[0][currentEvent][ch1] += result[0][currentEvent][ch1][currentCell];
                    }


                    result[0][currentEvent][ch2][sector * 16 + cell] =
                            *reinterpret_cast<unsigned short *>((group_pointer + 4 * cell + 1)) >> 4;
                    result[0][currentEvent][ch2][currentCell] = (result[0][currentEvent][ch2][currentCell] & 0b011111111111) | (~result[0][currentEvent][ch2][currentCell] & 0b100000000000);
                    if (zeroInd[ch2].first < currentCell && currentCell <= zeroInd[ch2].second) {
                        zero[0][currentEvent][ch2] += result[0][currentEvent][ch2][currentCell];
                    } else if (signalInd[ch2].first < currentCell && currentCell <= signalInd[ch2].second) {
                        ph_el[0][currentEvent][ch2] += result[0][currentEvent][ch2][currentCell];
                    }
                }
                group_pointer += 4 * 16;
            }
            zero[0][currentEvent][ch1] = config->offset - 1250 + RESOLUTION * zero[0][currentEvent][ch1] / (zeroInd[ch1].second - zeroInd[ch1].first);
            zero[0][currentEvent][ch2] = config->offset - 1250 + RESOLUTION * zero[0][currentEvent][ch2] / (zeroInd[ch2].second - zeroInd[ch2].first);

            ph_el[0][currentEvent][ch1] = config->offset - 1250 + RESOLUTION * ph_el[0][currentEvent][ch1] / (signalInd[ch1].second - signalInd[ch1].first);
            if (ph_el[0][currentEvent][ch1] > zero[0][currentEvent][ch1]){
                ph_el[0][currentEvent][ch1] -= zero[0][currentEvent][ch1];
            }else{
                ph_el[0][currentEvent][ch1] = 0;
            }

            ph_el[0][currentEvent][ch2] = config->offset - 1250 + RESOLUTION * ph_el[0][currentEvent][ch2] / (signalInd[ch2].second - signalInd[ch2].first);
            if (ph_el[0][currentEvent][ch2] > zero[0][currentEvent][ch2]){
                ph_el[0][currentEvent][ch2] -= zero[0][currentEvent][ch2];
            }else{
                ph_el[0][currentEvent][ch2] = 0;
            }
        }



        CAEN_DGTZ_ReadData(handles[1],CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, readoutBuffer, &readoutBufferSize);
        group_pointer = readoutBuffer + 16;

        for (int groupIdx = 0; groupIdx < MAX_V1743_GROUP_SIZE; groupIdx++) {
            size_t ch1 = 2 * groupIdx;
            size_t ch2 = ch1 + 1;
            for (unsigned short sector = 0; sector < 64; sector++) { // 64 sectors per 1024 cell page
                group_pointer += 4; // skip trash line, not mentioned in datasheet
                for (unsigned int cell = 0; cell < 16; cell++) {
                    currentCell = sector * 16 + cell;
                    result[1][currentEvent][ch1][currentCell] =
                            ((*reinterpret_cast<unsigned short *>(group_pointer + 4 * cell)) & 0x0FFF);
                    result[1][currentEvent][ch1][currentCell] = (result[1][currentEvent][ch1][currentCell] & 0b011111111111) | (~result[1][currentEvent][ch1][currentCell] & 0b100000000000);
                    if (zeroInd[ch1].first < currentCell && currentCell <= zeroInd[ch1].second) {
                        zero[1][currentEvent][ch1] += result[1][currentEvent][ch1][currentCell];
                    } else if (signalInd[ch1].first < currentCell && currentCell <= signalInd[ch1].second) {
                        ph_el[1][currentEvent][ch1] += result[1][currentEvent][ch1][currentCell];
                    }


                    result[1][currentEvent][ch2][sector * 16 + cell] =
                            *reinterpret_cast<unsigned short *>((group_pointer + 4 * cell + 1)) >> 4;
                    result[1][currentEvent][ch2][currentCell] = (result[1][currentEvent][ch2][currentCell] & 0b011111111111) | (~result[1][currentEvent][ch2][currentCell] & 0b100000000000);
                    if (zeroInd[ch2].first < currentCell && currentCell <= zeroInd[ch2].second) {
                        zero[1][currentEvent][ch2] += result[1][currentEvent][ch2][currentCell];
                    } else if (signalInd[ch2].first < currentCell && currentCell <= signalInd[ch2].second) {
                        ph_el[1][currentEvent][ch2] += result[1][currentEvent][ch2][currentCell];
                    }
                }
                group_pointer += 4 * 16;
            }
            zero[1][currentEvent][ch1] = config->offset - 1250 + RESOLUTION * zero[1][currentEvent][ch1] / (zeroInd[ch1].second - zeroInd[ch1].first);
            zero[1][currentEvent][ch2] = config->offset - 1250 + RESOLUTION * zero[1][currentEvent][ch2] / (zeroInd[ch2].second - zeroInd[ch2].first);

            ph_el[1][currentEvent][ch1] = config->offset - 1250 + RESOLUTION * ph_el[1][currentEvent][ch1] / (signalInd[ch1].second - signalInd[ch1].first);
            if (ph_el[1][currentEvent][ch1] > zero[1][currentEvent][ch1]){
                ph_el[1][currentEvent][ch1] -= zero[1][currentEvent][ch1];
            }else{
                ph_el[1][currentEvent][ch1] = 0;
            }
            ph_el[1][currentEvent][ch2] = config->offset - 1250 + RESOLUTION * ph_el[1][currentEvent][ch2] / (signalInd[ch2].second - signalInd[ch2].first);
            if (ph_el[1][currentEvent][ch2] > zero[1][currentEvent][ch2]){
                ph_el[1][currentEvent][ch2] -= zero[1][currentEvent][ch2];
            }else{
                ph_el[1][currentEvent][ch2] = 0;
            }
        }

        if(readoutBufferSize == EVT_SIZE){
            preload = false;
            processed[currentEvent]->count_down();
            currentEvent++;
            if(currentEvent >= SHOT_COUNT){
                std::cout << "CAEN 101+" << std::endl;
                return true;
            }
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
    unsigned long long mask = 1 << (link + 3);
    SetThreadAffinityMask(GetCurrentThread(), mask); //WINDOWS!!!
    std::cout << "Link " << link << " thread: " << SetThreadAffinityMask(GetCurrentThread(), mask) << std::endl; //WINDOWS!!!
    currentEvent = 0;
    preload = true;
}

