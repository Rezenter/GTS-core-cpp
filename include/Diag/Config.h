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
    std::string defaultIP = "172.16.12.130";
    std::string defaultPort = "80";
    std::string defaultHTMLRoot = "../html/";
    unsigned int defaultServerTheadCount = 4;

    //FastConfig fastConfig;
};


#endif //CONFIG_H
