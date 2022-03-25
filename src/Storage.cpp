//
// Created by ts_group on 09.07.2020.
//

#include "Storage.h"

Storage::Storage(Config &config) : config(config){

}

bool Storage::saveDischarge(const Json& data) const {
    std::stringstream path;
    if(config.isPlasma){
        path << config.plasmaPath << std::setw(5) << std::setfill('0') << config.plasmaShot << '/';
    }else{
        path << config.debugPath << std::setw(5) << std::setfill('0') << config.debugShot << '/';
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
        /*
        filename.str(std::string());
        filename << count << ".json";
        outFile.open(pathStr + filename.str());
        outFile << std::setw(2) << board << std::endl;
        outFile.close();
        */

        filename.str(std::string());
        filename << count++ << ".msgpk";
        outFile.open(pathStr + filename.str(), std::ios::out | std::ios::binary);
        for (const auto &e : Json::to_msgpack(board)) outFile << e;
        outFile.close();
    }
    std::cout << "Files written" << std::endl;
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
    return true;
}
