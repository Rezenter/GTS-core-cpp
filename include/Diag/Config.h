//
// Created by ts_group on 6/28/2020.
//

#ifndef CONFIG_H
#define CONFIG_H

#include <string>
//#include "FastADC/FastConfig.h"

class Config {
private:


public:
    std::string IP = "172.16.12.130";
    std::string Port = "80";
    std::string HTMLRoot = "../html/";
    unsigned int ServerTheadCount = 4;

    std::string plasmaPath = "d:/data/fastDump/plasma/";
    std::string debugPath = "d:/data/fastDump/debug/";
    std::string plasmaShotnPath = "z:/SHOTN.txt";
    std::string debugShotnPath = "d:/data/db/debug/SHOTN.txt";
    std::string gasPath = "d:/data/db/gas/";
    std::string configsPath = "d:/data/db/config/";
    std::string spectralPath = "d:/data/db/calibration/expected/";
    std::string absPath = "d:/data/db/calibration/abs/processed/";

    //FastConfig fastConfig;
};


#endif //CONFIG_H
