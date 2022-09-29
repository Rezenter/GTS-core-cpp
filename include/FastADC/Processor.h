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

    CAEN_DGTZ_X743_EVENT_t* decodedEvents[SHOT_COUNT];
    CAEN_DGTZ_X743_GROUP_t* group;

public:
    bool arm();
    bool disarm();
    explicit Processor(Config& config);
    ~Processor() override;

    std::array<std::array<std::array<double, PAGE_LENGTH>, CH_COUNT>, SHOT_COUNT> result;
    char encodedEvents[EVT_SIZE][SHOT_COUNT];
    std::atomic_int written;
    std::atomic_int processed;
    int handle;
};

#endif //GTS_CORE_PROCESSOR_H
