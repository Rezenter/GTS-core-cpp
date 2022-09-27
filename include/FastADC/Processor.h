//
// Created by ts_group on 27.09.2022.
//

#ifndef GTS_CORE_PROCESSOR_H
#define GTS_CORE_PROCESSOR_H

#include "Config.h"
#include "Stoppable.h"
#include <array>
#include "CAENDigitizer.h"

#define EVT_SIZE 34832
#define SHOT_COUNT 101
#define PAGE_LENGTH 1024
#define CH_COUNT 16

class Processor : public Stoppable{
private:
    Config& config;
    static const size_t boardCount = 2;
    CAEN_DGTZ_ErrorCode ret;
    bool payload() override;
    void beforePayload() override;
    void afterPayload() override;


    std::array<size_t, boardCount> counts;

    void process(size_t board);
    std::array<std::array<std::array<std::array<double, PAGE_LENGTH>, CH_COUNT>, SHOT_COUNT>, boardCount> result;
    std::array<std::array<CAEN_DGTZ_X743_EVENT_t*, SHOT_COUNT>, boardCount> decodedEvents;

public:
    bool arm();
    bool disarm();
    mutable std::mutex mutex;
    explicit Processor(Config& config);
    ~Processor() override;

    std::array<std::array<char[EVT_SIZE], SHOT_COUNT>, boardCount> encodedEvents;
    std::array<std::array<bool, SHOT_COUNT>, boardCount> eventFlags;
    std::array<int, boardCount> handles;
};

#endif //GTS_CORE_PROCESSOR_H
