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

    //FastConfig fastConfig;
};


#endif //CONFIG_H
