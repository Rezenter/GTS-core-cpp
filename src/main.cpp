//
// Created by ts_group on 6/28/2020.
//

#include "version.h"
//#include "include/FastADC/FastSystem.h"

#include <iostream>
#include <string>
#include "Server/server.hpp"

#include <filesystem>

int main([[maybe_unused]] int argc,[[maybe_unused]] char* argv[]) {
    std::cout << "TS fast acquisition, revision:" << REVISION << std::endl << std::flush;
    std::cout << std::filesystem::current_path() << '\n' <<std::flush;

    /*
    //FastADC legacy
    Config config;
    if(!config.load()){
        std::cout << "something went wrong during loading config." << std::endl;
    }
    FastSystem fs(config);
    while (!fs.exitRequested()){
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
*/

    //Server
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