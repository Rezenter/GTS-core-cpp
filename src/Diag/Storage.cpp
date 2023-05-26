//
// Created by ts_group on 09.07.2020.
//

#include "include/Diag/Storage.h"
#include "include/Diag/Diag.h"
#include <io.h>
#include <iomanip>
#include <fstream>
#include <filesystem>

//debug
#include <iostream>
//debug


Json Storage::getConfigsNames() {
    Json res = {};

    /*
    //get configs
    res["configs"] = Json::array();
    std::string filename;
    for (const auto & entry : std::filesystem::directory_iterator(config->configsPath)) {
        filename = entry.path().filename().string();
        res["configs"].push_back(filename.substr(0, filename.find_last_of('.')));
    }

    //get spectral
    res["spectral"] = Json::array();
    for (const auto & entry : std::filesystem::directory_iterator(config->spectralPath)) {
        filename = entry.path().filename().string();
        res["spectral"].push_back(filename.substr(0, filename.find_last_of('.')));
    }

    //get abs
    res["abs"] = Json::array();
    for (const auto & entry : std::filesystem::directory_iterator(config->absPath)) {
        filename = entry.path().filename().string();
        res["abs"].push_back(filename.substr(0, filename.find_last_of('.')));
    }


    //get gas
    res["gas"] = Json::array();
    for (const auto & entry : std::filesystem::directory_iterator(config->gasPath)) {
        filename = entry.path().filename().string();
        res["gas"].push_back(filename.substr(0, filename.find_last_of('.')));
    }
     */
    return res;
}
