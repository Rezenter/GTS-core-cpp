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

Json Storage::getConfigsNames() const {
    Json res = {};

    //get configs
    res["configs"] = Json::array();
    std::string filename;
    for (const auto & entry : std::filesystem::directory_iterator(parent.config.configsPath)) {
        if(entry.path().extension() == ".json"){
            filename = entry.path().filename().string();
            res["configs"].push_back(filename.substr(0, filename.find_last_of('.')));
        }
    }

    /*
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

Json Storage::loadConfig(const std::string& filename){
    Json res = {{"ok",  false},
                {"err", filename}};

    parent.config.config_ready = false;

    std::filesystem::path path = parent.config.configsPath + filename + extension_json;
    if(!std::filesystem::is_regular_file(path)){
        res["ok"] = false;
        res["err"] = "no such file " + path.string();
        return res;
    }

    std::ifstream file;
    file.open(path);
    Json candidate = Json::parse(file);
    file.close();

    // check & fill parent.config

    try{
        if(!candidate.contains("type")){
            res["err"] = "no 'type' in config!";
            return res;
        }
        if(candidate.at("type") != "configuration_cpp"){
            res["err"] = "wrong config type! expected 'configuration_cpp'";
            return res;
        }


        //candidate.is_number();


    } catch(Json::exception& e){
        res["err"] = e.what();
        return res;
    }

    parent.config.config_ready = true;
    res["ok"] = true;
    return res;
}
