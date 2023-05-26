//
// Created by ts_group on 09.07.2020.
//

#ifndef DIAG_STORAGE_H
#define DIAG_STORAGE_H

#include "json.hpp"


using Json = nlohmann::json;

class Diag;

class Storage {
private:
    Diag& parent;

public:
    Storage(Diag& parent): parent(parent){} ;
    //explicit Storage();

    Json getConfigsNames();
};


#endif //DIAG_STORAGE_H
