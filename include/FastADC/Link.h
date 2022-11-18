//
// Created by user on 6/18/2020.
//
#ifndef LINK_H
#define LINK_H

#include "CAENDigitizer.h"
#include "Config.h"
#include "Stoppable.h"
#include <vector>
#include <array>
#include <latch>

#define CAEN_USE_DIGITIZERS
#define IGNORE_DPP_DEPRECATED

#define MAX_TRANSFER 1 //maximum events per transaction
#define EVT_SIZE 34832
#define SHOT_COUNT 101
#define PAGE_LENGTH 1024
#define CH_COUNT 16
#define RESOLUTION 0.6103516

union Timestamp{
    char bytes[8];
    unsigned long long int integer;
};

class Link final : public Stoppable{
private:
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

    Config* config;
    int link; //optical link number

    CAEN_DGTZ_ErrorCode ret;
    int	handles[2];
    Timestamp timestampConverter;

    bool payload() override;
    void beforePayload() override;
    void afterPayload() override;

    bool initialized = false;
    CAEN_DGTZ_BoardInfo_t boardInfo;
    size_t currentEvent = 0;
    unsigned int short currentCell = 0;
    bool preload = true;

public:
    explicit Link(unsigned short link, std::array<std::latch*, SHOT_COUNT>& processed) : link(link), processed(processed){};
    ~Link() override;
    bool isAlive();
    int init(Config& config);
    bool arm();
    bool disarm();
    int serials[2] = {0, 0};

    char* readoutBuffer;
    uint32_t readoutBufferSize;
    std::array<std::array<std::array<unsigned short, PAGE_LENGTH>, CH_COUNT>, SHOT_COUNT> result[2];
    std::array<std::array<unsigned int, CH_COUNT>, SHOT_COUNT> zero [2];
    std::array<std::array<unsigned int, CH_COUNT>, SHOT_COUNT> ph_el [2];
    std::array<unsigned long long int, SHOT_COUNT> times;
    std::array<std::latch*, SHOT_COUNT>& processed;
};

#endif //LINK_H
