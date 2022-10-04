//
// Created by ts_group on 27.09.2022.
//

#ifndef GTS_CORE_PROCESSOR_H
#define GTS_CORE_PROCESSOR_H

#include "Config.h"
#include "Stoppable.h"
#include <array>
#include "CAENDigitizer.h"
#include <atomic>
#include <semaphore>
#include <chrono>
#include <latch>
#include <mutex>

#define EVT_SIZE 34832
#define SHOT_COUNT 101
#define PAGE_LENGTH 1024
#define CH_COUNT 16

class Processor : public Stoppable{
private:
    Config& config;
    CAEN_DGTZ_ErrorCode ret;
    bool payload() override;
    void beforePayload() override;
    void afterPayload() override;
    constexpr static const double resolution = 2500.0 / 4096;


    CAEN_DGTZ_X743_GROUP_t* group;

    size_t current_index = 0;


    constexpr const static std::pair<size_t, size_t> zeroInd[16] = {
            {100, 200},
            {10, 500},
            {10, 500},
            {10, 500},
            {10, 500},
            {10, 500},
            {10, 500},
            {10, 500},
            {10, 500},
            {10, 500},
            {10, 500},
            {10, 500},
            {10, 500},
            {10, 500},
            {10, 500},
            {10, 500}
    };
    constexpr const static std::pair<size_t, size_t> signalInd[16] = {
            {240, 400},
            {520, 670},
            {520, 670},
            {520, 670},
            {520, 670},
            {520, 670},
            {520, 670},
            {520, 670},
            {520, 670},
            {520, 670},
            {520, 670},
            {520, 670},
            {520, 670},
            {520, 670},
            {520, 670},
            {520, 670}
    };
    std::array<std::latch*, SHOT_COUNT>& processed;

public:
    bool arm();
    bool disarm();
    explicit Processor(Config& config, std::array<std::latch*, SHOT_COUNT>& processed);
    ~Processor() override;

    CAEN_DGTZ_X743_EVENT_t* decodedEvents[SHOT_COUNT];
    std::array<std::array<std::array<double, PAGE_LENGTH>, CH_COUNT>, SHOT_COUNT> result;
    std::array<std::array<double, CH_COUNT>, SHOT_COUNT> zero;
    std::array<std::array<double, CH_COUNT>, SHOT_COUNT> ph_el;
    std::array<double, SHOT_COUNT> times;
    std::counting_semaphore<SHOT_COUNT> *written;
    int handle;
};

#endif //GTS_CORE_PROCESSOR_H
