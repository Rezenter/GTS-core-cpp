//
// Created by ts_group on 09.07.2020.
//

#ifndef CAEN743_STORAGE_H
#define CAEN743_STORAGE_H

#include "Config.h"
#include "json.hpp"
#include <io.h>
#include <iomanip>
#include <fstream>
#include <filesystem>


//debug
#include <iostream>
//debug

using Json = nlohmann::json;

class Storage {
private:
    Config& config;

public:
    explicit Storage(Config& config);

    bool saveDischarge(const Json& data) const;
    [[nodiscard]] bool isAlive() const;

    Json getSavesNames();
};


#endif //CAEN743_STORAGE_H
