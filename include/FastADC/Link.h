//
// Created by user on 6/18/2020.
//
#ifndef LINK_H
#define LINK_H

#include "CAENDigitizer.h"
#include "Config.h"
#include "Stoppable.h"
#include "Processor.h"
#include <vector>
#include <array>

#define CAEN_USE_DIGITIZERS
#define IGNORE_DPP_DEPRECATED

#define MAX_TRANSFER 5 //maximum events per transaction

class Link final : public Stoppable{
private:
    Config* config;
    int link; //optical link number

    CAEN_DGTZ_ErrorCode ret;
    int	handles[2];


    bool payload() override;
    void beforePayload() override;
    void afterPayload() override;

    bool initialized = false;
    uint32_t numEvents;
    CAEN_DGTZ_BoardInfo_t boardInfo;
    int currentEvent[2] = {0, 0};

public:
    Link(unsigned short link, Processor* processor1, Processor* processor2) :
        link(link), processors{processor1, processor2}{};
    ~Link() override;
    bool isAlive();
    int init(Config& config);
    bool arm();
    bool disarm();
    int serials[2] = {0, 0};
    Processor* processors[2];
};

#endif //LINK_H
