//
// Created by ts_group on 09.07.2020.
//

#ifndef CAEN743_STORAGE_H
#define CAEN743_STORAGE_H

#include "FastConfig.h"
#include "json.hpp"
#include <io.h>
#include <iomanip>
#include <fstream>
#include <filesystem>


//debug
#include <iostream>
//debug

using Json = nlohmann::json;

class Storage_old {
private:
    FastConfig* config;

public:
    Storage_old(){};
    explicit Storage_old(FastConfig* config);

    bool saveDischarge(const Json& data) const;
    [[nodiscard]] bool isAlive() const;

    Json getConfigsNames();
    Json getGas(const std::string& name);
    bool saveGas(const std::string& name, const Json& prog);
};


#endif //CAEN743_STORAGE_H