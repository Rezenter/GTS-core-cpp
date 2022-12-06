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
    std::cout << "Process affinity: " << ' ' << SetProcessAffinityMask(GetCurrentProcess(), 0b001111111110) << std::endl; //WINDOWS!!!
    std::cout << "process realtime: " << ' ' << SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS) << std::endl; //WINDOWS!!!

    unsigned long long mask = 0b001111000000;
    SetThreadAffinityMask(GetCurrentThread(), mask); //WINDOWS!!!
    std::cout << "Main thread: " << ' ' << SetThreadAffinityMask(GetCurrentThread(), mask) << std::endl; //WINDOWS!!!
    try{
        http::server3::server s("172.16.12.130", "8080", "../html/", 4); // 4 = num_threads
        s.run();
    }
    catch (std::exception& e){
        std::cerr << "exception: " << e.what() << "\n";
    }

    std::cout << "\nNormal exit." << std::endl << std::flush;
    return 0;
}