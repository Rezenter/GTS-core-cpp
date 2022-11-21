//
// Created by ts_group on 6/28/2020.
//

#include "version.h"

#include <iostream>
#include <string>
#include "Server/server.hpp"
#include <filesystem>

int main([[maybe_unused]] int argc,[[maybe_unused]] char* argv[]) {
    std::cout << "TS C++ server, revision:" << REVISION << std::endl << std::flush;
    std::cout << std::filesystem::current_path() << '\n' <<std::flush;
    unsigned long long mask = 1 << 1;
    SetThreadAffinityMask(GetCurrentThread(), mask); //WINDOWS!!!
    std::cout << "Main thread: " << ' ' << SetThreadAffinityMask(GetCurrentThread(), mask) << std::endl; //WINDOWS!!!
    try{
        http::server::server s("172.16.12.130", "8080", "../html/");
        s.run();
    }
    catch (std::exception& e){
        std::cerr << "exception: " << e.what() << "\n";
    }

    std::cout << "\nNormal exit." << std::endl << std::flush;
    return 0;
}