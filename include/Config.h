//
// Created by ts_group on 6/28/2020.
//

#ifndef CAEN743_CONFIG_H
#define CAEN743_CONFIG_H

#define MAX_CAENS 8

#include <string>
#include "CAENDigitizerType.h"
#include "json.hpp"
#include <filesystem>
#include <fstream>

using Json = nlohmann::json;

class Config {
private:

public:
    [[nodiscard]] std::string freqStr() const;
    //crate
    unsigned int caenCount = 4;
    unsigned int links[MAX_CAENS];
    unsigned int nodes[MAX_CAENS];

    //experiment
    int acquisitionTime = 1; //seconds
    unsigned int debugShot = 0;
    unsigned int plasmaShot = 0;
    unsigned int globusShot = 0;
    bool isPlasma = false;
    Json aux_args = {};

    //caen
    CAEN_DGTZ_SAMFrequency_t frequency = CAEN_DGTZ_SAM_3_2GHz; // 1_6G, 800M, 400M
    unsigned int recordLength = 1024;
    unsigned int triggerDelay = 18; // *16 cells
    float offset = 1100.0; // mv
    float triggerThreshold = 300.0; // mv
    uint16_t offsetADC = 0x7FFF;
    uint16_t triggerThresholdADC = 0x7FFF;

    //connection
    unsigned int connectionTimeout = 1; // seconds between connections
    unsigned int commandTimeout = 100; // milliseconds between command readout attempts
    unsigned int connectionDeadTime = 120; // seconds before connection close
    unsigned int messagePoolingInterval = 100; // milliseconds between message reading

    //storage
    std::string plasmaPath = "d:/data/fastDump/plasma/";
    std::string debugPath = "d:/data/fastDump/debug/";
    std::string logPath = "d:/data/fastDump/logs/";
    unsigned int verboseLevel = 0;

    //bool load(std::string path = "d:/data/db/config.json");
    bool load(std::string path = "d:/data/db/config_debug.json");
};


#endif //CAEN743_CONFIG_H
