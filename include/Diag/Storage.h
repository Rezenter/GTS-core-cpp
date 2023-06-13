//
// Created by ts_group on 09.07.2020.
//

#ifndef DIAG_STORAGE_H
#define DIAG_STORAGE_H

#include "json.hpp"

#define extension_json ".json"

using Json = nlohmann::json;

class Diag;

class Storage {
private:
    Diag& parent;

public:
    Storage(Diag& parent): parent(parent){} ;
    //explicit Storage();

    Json getConfigsNames() const;
    Json loadConfig(const std::string& filename);
};


#endif //DIAG_STORAGE_H
