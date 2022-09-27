//
// Created by user on 6/18/2020.
//
#ifndef CAEN743_CAEN743_H
#define CAEN743_CAEN743_H

#include "CAENDigitizer.h"
#include "Config.h"
#include "Stoppable.h"
#include "Processor.h"
#include "json.hpp"
#include <vector>
#include <array>

#define CAEN_USE_DIGITIZERS
#define IGNORE_DPP_DEPRECATED

#define MAX_TRANSFER 10 //maximum events per transaction
#define MASTER 0 // address of the master board

using Json = nlohmann::json;

typedef enum CAEN_ErrorCode {
    CAEN_Success = 0, //ok
    CAEN_Error_Connection = -1, // Can't open digitizer
}CAEN_ErrorCode;

class CAEN743 final : public Stoppable{
private:
    Config* config;
    static unsigned char caenCount;
    const unsigned char address; //optical link number
    const unsigned char chain_node; //position in daisy-chain of optical link

    CAEN_DGTZ_ErrorCode ret;
    int	handle;

    char* buffer;
    uint32_t bufferSize;

    std::vector<char*> events;

    CAEN_DGTZ_BoardInfo_t boardInfo;

    bool payload() override;
    void beforePayload() override;
    void afterPayload() override;

    bool initialized = false;
    uint32_t numEvents;
    uint32_t counter = 0;
    CAEN_DGTZ_EventInfo_t eventInfo;

    Processor* processor = nullptr;
    char* eventEncoded = nullptr;
    Json results = Json::array();
    size_t currentEvent = 0;

public:
    CAEN743(unsigned int address, unsigned int node, Processor* processor) :
        address(address), chain_node(node), processor(processor){caenCount++;};
    ~CAEN743() override;
    bool isAlive();
    int init(Config& config);
    bool arm();
    bool disarm();
    Json waitTillProcessed();
    bool releaseMemory();
    [[nodiscard]] int getSerial() const{return boardInfo.SerialNumber;};

    bool eventReady = false;
};

#endif //CAEN743_CAEN743_H
