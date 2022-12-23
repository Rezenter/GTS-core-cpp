//
// Created by ts_group on 20.12.2022.
//

#include "SlowADC/SlowSubsystem.h"
#include <process.h>


SlowSubsystem::SlowSubsystem(){

    //spawnl(P_NOWAIT, "copy 1", "pscp");
}

SlowSubsystem::~SlowSubsystem(){

}

Json SlowSubsystem::requestHandler(Json req){
    Json resp = {{"ok", true}};

    if(req.contains("reqtype")){
        if(req.at("reqtype") == "reboot"){
            bool connected = connect();
            if(connected){
                resp["ok"] = true;
            }else {
                resp["ok"] = false;
                resp["err"] = "Not implemented!";
            }
        }else
        if(req.at("reqtype") == "arm"){
            resp["ok"] = false;
            resp["err"] = "Not implemented!";
        }else
        if(req.at("reqtype") == "disarm"){
            resp["ok"] = false;
            resp["err"] = "Not implemented!";
        }else
        if(req.at("reqtype") == "state"){
            resp["ok"] = false;
            resp["err"] = "Not implemented!";
        }
    }else{
        resp["ok"] = false;
        resp["err"] = "reqtype is not listed";
    }
    return resp;
}

bool SlowSubsystem::connect() {
    std::cout << "SlowADC connect" << std::endl;
    try{
        tcp::socket socket(io_context);
        tcp::resolver resolver(io_context);
        boost::asio::connect(socket, resolver.resolve(addresses[0], PORT));

        std::cout << "SlowADC write" << std::endl;
        boost::asio::write(socket, boost::asio::buffer(req_status, std::strlen(req_status)));

        char reply[MAX_LENGTH];
        size_t request_length = 0;
        std::cout << "SlowADC read" << std::endl;
        size_t reply_length = boost::asio::read(socket, boost::asio::buffer(reply, request_length));
        std::cout << "Reply ok: " << (strcmp(reply, status_ready) == 0) << std::endl;
    }
    catch (std::exception& e){
        std::cerr << "Exception: " << e.what() << "\n";
        return false;
    }
    std::cout << "SlowADC connect OK" << std::endl;
    return true;
}