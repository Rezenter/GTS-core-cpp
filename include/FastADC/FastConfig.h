//
// Created by ts_group on 6/28/2020.
//

#ifndef CAEN743_CONFIG_H
#define CAEN743_CONFIG_H

#define MAX_CAENS 8
#define MAX_LINKS 4

#include <string>
#include "CAENDigitizerType.h"
#include "json.hpp"
#include <filesystem>
#include <fstream>

using Json = nlohmann::json;

class FastConfig {
private:

public:
    [[nodiscard]] std::string freqStr() const;
    //crate
    unsigned int caenCount = 8;
    unsigned int linkCount = 4;
    unsigned int links[MAX_CAENS];

    //experiment
    unsigned int debugShot = 0;
    unsigned int plasmaShot = 0;
    bool isPlasma = false;
    Json aux_args = {};

    //caen
    CAEN_DGTZ_SAMFrequency_t frequency = CAEN_DGTZ_SAM_3_2GHz; // 1_6G, 800M, 400M
    unsigned int recordLength = 1024;
    unsigned int triggerDelay = 18; // *16 cells
    float offset = 1100.0; // mv
    uint16_t offsetADC = 0x7FFF;  // CALCULATE!!!!

    //storage
    std::string plasmaPath = "d:/data/fastDump/plasma/";
    std::string debugPath = "d:/data/fastDump/debug/";
    std::string plasmaShotnPath = "z:/SHOTN.txt";
    std::string debugShotnPath = "d:/data/db/debug/SHOTN.txt";
    std::string gasPath = "d:/data/db/gas/";
    std::string configsPath = "d:/data/db/config/";
    std::string spectralPath = "d:/data/db/calibration/expected/";
    std::string absPath = "d:/data/db/calibration/abs/processed/";

    bool load(std::string path = "d:/data/db/config_cpp.json");
};


#endif //CAEN743_CONFIG_H
