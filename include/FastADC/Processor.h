//
// Created by ts_group on 27.09.2022.
//

#ifndef GTS_CORE_PROCESSOR_H
#define GTS_CORE_PROCESSOR_H

#include "Config.h"
#include "Stoppable.h"
#include <array>
#include "CAENDigitizer.h"
#include <semaphore>
#include <chrono>
#include <latch>

#define EVT_SIZE 34832
#define SHOT_COUNT 101
#define PAGE_LENGTH 1024
#define CH_COUNT 16

union Timestamp{
    char bytes[8];
    unsigned long long int integer;
};

class Processor : public Stoppable{
private:
    Config& config;
    CAEN_DGTZ_ErrorCode ret;
    bool payload() override;
    void beforePayload() override;
    void afterPayload() override;
    constexpr static const double resolution = 2500.0 / 4096;
    Timestamp timestampConverter;
    unsigned int short currentCell = 0;

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

    std::array<std::array<std::array<unsigned short, PAGE_LENGTH>, CH_COUNT>, SHOT_COUNT> result;
    std::array<std::array<unsigned int, CH_COUNT>, SHOT_COUNT> zero;
    std::array<std::array<unsigned int, CH_COUNT>, SHOT_COUNT> ph_el;
    std::array<double, SHOT_COUNT> times;
    std::counting_semaphore<SHOT_COUNT> *written;
    std::array<std::chrono::time_point<std::chrono::steady_clock>, SHOT_COUNT> tProcessed;

    int handle;
    std::array<char*, SHOT_COUNT> readoutBuffer;
    std::array<uint32_t, SHOT_COUNT> readoutBufferSize;
};

#endif //GTS_CORE_PROCESSOR_H
