//
// Created by user on 6/18/2020.
//

#include <iostream>
#include <iomanip>
#include <sstream>
#include "json.hpp"

using Json = nlohmann::json;

void delay(){
    for(int i  = 0; i < 100000000; i++){
        double fuck = 1.0/i;
    }
}

int main(int argc, char* argv[]){
    std::cout << "let the test begin\n" << std::endl << std::flush;

    char text[] = R"(
     {
         "Image": {
             "Width":  800,
             "Height": 600,
             "Title":  "View from 15th Floor",
             "Thumbnail": {
                 "Url":    "http://www.example.com/image/481989943",
                 "Height": 125,
                 "Width":  100
             },
             "Animated" : false,
             "IDs": [116, 943, 234, 38793]
         }
     }
     )";

    // parse and serialize JSON
    Json j_complete = Json::parse(text);
    std::cout << std::setw(4) << j_complete << "\n\n";

    std::cout << "\ntotally clean" << std::endl << std::flush;
    delay();
    return 0;
}
