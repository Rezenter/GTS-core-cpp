//
// Created by ts_group on 15.12.2022.
//

#include <iostream>
#include <string>
#include "SHT/compress.hpp"
#include <fstream>
#include <filesystem>
#include "json.hpp"

using Json = nlohmann::json;

int main([[maybe_unused]] int argc,[[maybe_unused]] char* argv[]) {
    //std::cout << "Slow ADC to sht" << std::endl << std::flush;

    std::string arg = argv[1];
    int shotn = std::stoi(arg);

    std::string binary_foldername = argv[2];
    std::string out_foldername = argv[3];

    std::string configFilename = argv[4];
    std::ifstream configFile;
    configFile.open(configFilename);
    Json config = Json::parse(configFile);
    configFile.close();

    std::string outFilename = out_foldername + "passive" + std::to_string(shotn) + ".SHT";
    std::ofstream outFile;
    outFile.open(outFilename, std::ios::out | std::ios::binary);
    if (outFile.is_open()) {
        std::filesystem::path binary_path(binary_foldername);
        binary_path.append("sht" + std::to_string(shotn));
        //check path for all 4 files
        //std::cout << binary_path << std::endl;


        Out out = packADC(binary_path, config);
        outFile.write(out.point, out.size);
        outFile.close();
        innerFreeOut();
    } else {
        std::cout << "Unable to open file" << std::endl;
    }

    //WARNING!!! resource are not released!

    //std::cout << "Normal exit." << std::endl << std::flush;
    return 0;
}