//
// Created by ts_group on 09.07.2020.
//

#include "include/FastADC/Storage.h"

Storage::Storage(Config &config) : config(config){

}

bool Storage::saveDischarge(const Json& data) const {
    std::stringstream path;
    if(config.isPlasma){
        path << config.plasmaPath << std::setw(5) << std::setfill('0') << config.plasmaShot << '/';
    }else{
        path << config.debugPath << std::setw(5) << std::setfill('0') << config.debugShot << '/';
        std::ofstream debugShotnFile(config.debugShotnPath);
        debugShotnFile << (config.debugShot + 1);
        debugShotnFile.close();
    }
    std::string pathStr = path.str();
    std::filesystem::create_directory(pathStr);
    std::ofstream outFile;

    outFile.open(pathStr + "header.json");
    outFile << std::setw(2) << data["header"] << std::endl;
    outFile.close();

    int count = 0;
    std::stringstream filename;
    for(auto& board : data["boards"]){
        filename.str(std::string());
        filename << count++ << ".msgpk";
        outFile.open(pathStr + filename.str(), std::ios::out | std::ios::binary);
        for (const auto &e : Json::to_msgpack(board)) outFile << e;
        outFile.close();
    }
    std::cout << "Files written: " << pathStr << std::endl;
    return true;
}

bool Storage::isAlive() const {
    if(!std::filesystem::is_directory(config.plasmaPath)){
        std::cout << "Directory" << config.plasmaPath << "for plasma shots not found." << std::endl;
        return false;
    }
    if(!std::filesystem::is_directory(config.debugPath)){
        std::cout << "Directory" << config.debugPath << "for debug shots not found." << std::endl;
        return false;
    }
    if(!std::filesystem::is_regular_file(config.plasmaShotnPath)){
        std::cout << "File" << config.plasmaShotnPath << "for plasma shotn not found." << std::endl;
        return false;
    }
    if(!std::filesystem::is_regular_file(config.debugShotnPath)){
        std::cout << "File" << config.debugShotnPath << "for debug shotn not found." << std::endl;
        return false;
    }
    std::ifstream plasmaShotnFile(config.plasmaShotnPath);
    plasmaShotnFile >> config.plasmaShot;
    plasmaShotnFile.close();

    std::ifstream debugShotnFile(config.debugShotnPath);
    debugShotnFile >> config.debugShot;
    debugShotnFile.close();
    return true;
}

Json Storage::getConfigsNames() {
    Json res = {};

    //get configs
    res["configs"] = Json::array();
    for (const auto & entry : std::filesystem::directory_iterator(config.configsPath)) {
        res["configs"].push_back(entry.path().filename());
    }

    //get spectral
    res["spectral"] = Json::array();
    for (const auto & entry : std::filesystem::directory_iterator(config.spectralPath)) {
        res["spectral"].push_back(entry.path().filename());
    }

    //get abs
    res["abs"] = Json::array();
    for (const auto & entry : std::filesystem::directory_iterator(config.absPath)) {
        res["abs"].push_back(entry.path().filename());
    }


    //get gas
    res["gas"] = Json::array();
    for (const auto & entry : std::filesystem::directory_iterator(config.gasPath)) {
        res["gas"].push_back(entry.path().filename());

    }
    return res;
}

Json Storage::getGas(const std::string& name) {
    Json res = {};

    std::filesystem::path path = config.gasPath + name;
    if(!std::filesystem::is_regular_file(path)){
        res["ok"] = false;
        res["err"] = "no such file " + path.string();
        return res;
    }

    std::ifstream file;
    file.open(path);
    res["data"] = Json::parse(file);
    file.close();

    res["ok"] = true;
    return res;
}

bool Storage::saveGas(const std::string &name, const Json& prog) {
    std::filesystem::path path = config.gasPath + name + ".json";
    if(std::filesystem::is_regular_file(path)){
        return false;
    }

    std::ofstream outFile;
    outFile.open(path, std::ios::out);
    outFile << prog.dump(2);
    outFile.close();
    return true;
}