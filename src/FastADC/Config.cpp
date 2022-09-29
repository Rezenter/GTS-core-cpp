//
// Created by ts_group on 6/28/2020.
//

#include "include/FastADC/Config.h"

#include "iostream"

bool Config::load(std::string path) {
    if(!std::filesystem::exists(path)){
        return false;
    }
    std::ifstream configFile;
    configFile.open(path);
    Json config = Json::parse(configFile);
    configFile.close();
    Json section;
    std::string key;
    bool flag = true;

    key = "crate";
    if(config.contains(key)){
        section = config[key];
        key = "caens";
        if(section.contains(key)){
            section = section[key];
            unsigned int candidate = section.size();
            if(candidate != 0 && candidate <= MAX_CAENS){
                caenCount = candidate;
                for(unsigned int i = 0; i < caenCount; i++){
                    Json caen = section[i];
                    if(caen.contains("link") && caen.contains("node")){
                        if(!(caen["link"] >= 0 && caen["link"] <= 4) || !(caen["node"] >= 0 && caen["node"] <= MAX_CAENS)){
                            flag = false;
                            std::cout << "Wrong caen link/node value!" << std::endl;
                            break;
                        }
                        links[i] = caen["link"];
                        nodes[i] = caen["node"];
                    }else{
                        flag = false;
                        std::cout << "Wrong caen link/node description!" << std::endl;
                        break;
                    }
                }
            }else{
                flag = false;
                std::cout << "Wrong value for '" << key << "' = " << candidate << '.' << std::endl;
            }
        }else{
            flag = false;
            std::cout << "Config file missing '" << key << "', using default." << std::endl;
        }
    }else{
        flag = false;
        std::cout << "Config file missing '" << key << "' section, using defaults." << std::endl;
    }


    key = "experiment";
    if(config.contains(key)){
        section = config[key];

        key = "eventCount";
        if(section.contains(key)){
            shotCount = section[key];
        }else{
            flag = false;
            std::cout << "Config file missing '" << key << "', using default." << std::endl;
        }
    }else{
        flag = false;
        std::cout << "Config file missing '" << key << "' section, using defaults." << std::endl;
    }

    key = "caen";
    if(config.contains(key)){
        section = config[key];
        key = "recordLength";
        if(section.contains(key)){
            unsigned int candidate = section[key];
            if(candidate % 16 == 0 && candidate <= 1024){
                recordLength = candidate;
            }else{
                flag = false;
                std::cout << "Wrong value for '" << key << "' = " << candidate << '.' << std::endl;
            }
        }else{
            flag = false;
            std::cout << "Config file missing '" << key << "', using default." << std::endl;
        }

        key = "triggerDelay";
        if(section.contains(key)){
            unsigned int candidate = section[key];
            if(candidate <= 64){
                triggerDelay = candidate;
            }else{
                flag = false;
                std::cout << "Wrong value for '" << key << "' = " << candidate << '.' << std::endl;
            }
        }else{
            flag = false;
            std::cout << "Config file missing '" << key << "', using default." << std::endl;
        }

        key = "frequency";
        if(section.contains(key)){
            std::string candidate = section[key];
            if(candidate == "3.2"){
                frequency = CAEN_DGTZ_SAM_3_2GHz;
            }else if(candidate == "1.6"){
                frequency = CAEN_DGTZ_SAM_1_6GHz;
            }else if(candidate == "0.8"){
                frequency = CAEN_DGTZ_SAM_800MHz;
            }else if(candidate == "0.4"){
                frequency = CAEN_DGTZ_SAM_400MHz;
            }else{
                flag = false;
                std::cout << "Wrong value for '" << key << "' = " << candidate << '.' << std::endl;
            }
        }else{
            flag = false;
            std::cout << "Config file missing '" << key << "', using default." << std::endl;
        }

        key = "offset";
        if(section.contains(key)){
            float candidate = section[key];
            if(-1250 <= candidate && candidate <= 1250){
                offset = candidate;
                offsetADC = 0x7FFF - (offset / 2500) * 0xFFFF;
            }else{
                flag = false;
                std::cout << "Wrong value for '" << key << "' = " << candidate << '.' << std::endl;
            }
        }else{
            flag = false;
            std::cout << "Config file missing '" << key << "', using default." << std::endl;
        }

        key = "triggerThreshold";
        if(section.contains(key)){
            float candidate = section[key];
            if(-1250 + offset <= candidate && candidate <= 1250 + offset){
                triggerThreshold = candidate;
                triggerThresholdADC = (1250 + offset - triggerThreshold) * 0xFFFF / 2500;
            }else{
                flag = false;
                std::cout << "Wrong value for '" << key << "' = " << candidate << '.' << std::endl;
            }
        }else{
            flag = false;
            std::cout << "Config file missing '" << key << "', using default." << std::endl;
        }
    }else{
        flag = false;
        std::cout << "Config file missing '" << key << "' section, using defaults." << std::endl;
    }

    key = "storage";
    if(config.contains(key)){
        section = config[key];
        key = "plasmaPath";
        if(section.contains(key)){
            std::string candidate = section[key];
            if(std::filesystem::exists(candidate)){
                plasmaPath = candidate;
            }else{
                flag = false;
                std::cout << "Wrong value for '" << key << "' = " << candidate << '.' << std::endl;
            }
        }else{
            flag = false;
            std::cout << "Config file missing '" << key << "', using default." << std::endl;
        }

        key = "debugPath";
        if(section.contains(key)){
            std::string candidate = section[key];
            if(std::filesystem::exists(candidate)){
                debugPath = candidate;
            }else{
                flag = false;
                std::cout << "Wrong value for '" << key << "' = " << candidate << '.' << std::endl;
            }
        }else{
            flag = false;
            std::cout << "Config file missing '" << key << "', using default." << std::endl;
        }

        key = "plasmaShotnPath";
        if(section.contains(key)){
            std::string candidate = section[key];
            if(std::filesystem::exists(candidate)){
                plasmaShotnPath = candidate;
            }else{
                flag = false;
                std::cout << "Wrong value for '" << key << "' = " << candidate << '.' << std::endl;
            }
        }else{
            flag = false;
            std::cout << "Config file missing '" << key << "', using default." << std::endl;
        }

        key = "debugShotnPath";
        if(section.contains(key)){
            std::string candidate = section[key];
            if(std::filesystem::exists(candidate)){
                debugShotnPath = candidate;
            }else{
                flag = false;
                std::cout << "Wrong value for '" << key << "' = " << candidate << '.' << std::endl;
            }
        }else{
            flag = false;
            std::cout << "Config file missing '" << key << "', using default." << std::endl;
        }
    }else{
        flag = false;
        std::cout << "Config file missing '" << key << "' section, using defaults." << std::endl;
    }

    return flag;
}

std::string Config::freqStr() const {
    switch (frequency) {
        case CAEN_DGTZ_SAM_3_2GHz:
            return "3.2";
        case CAEN_DGTZ_SAM_1_6GHz:
            return "1.6";
        case CAEN_DGTZ_SAM_800MHz:
            return "0.8";
        case CAEN_DGTZ_SAM_400MHz:
            return "0.4";
        default:
            return "0.0";
    }
}
