//
// Created by ts_group on 6/28/2020.
//


#include "FastSystem.h" //should be before "root"
#include "root.h"

void delay(){
    for(int i  = 0; i < 100000000; i++){
        double fuck = 1.0/i;
    }
}

int main(int argc, char* argv[]) {
    std::cout << "TS fast acquisition, revision:" << REVISION << "\n\n\n" << std::endl << std::flush;
    delay();
    std::cout << std::endl;

    Config config;

    std::cout << "loading config... " << std::endl;
    if(config.load()){
        std::cout << "configuration ok." << std::endl;
    }else{
        std::cout << "something went wrong during loading calibration." << std::endl;
    }

    std::cout << "Voltage range: [" << config.offset - 1250 << ", " << config.offset + 1250 << "] mv." << std::endl;
    //std::cout << "Trigger level = " << config.triggerThreshold << " mv." << std::endl; // ch0 trigger should not be used
    //std::cout << "offset ADC = " << config.offsetADC << ", trigger ADC = " << config.triggerThresholdADC << std::endl;

    FastSystem fs(config);

    while (!fs.exitRequested()){
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "\nNormal exit." << std::endl << std::flush;
    delay();
}