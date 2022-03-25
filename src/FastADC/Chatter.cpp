//
// Created by ts_group on 10.07.2020.
//

#include "include/FastADC/Chatter.h"

bool Chatter::init(Config &config) {
    //std::cout << "init chatter" << std::endl;
    this->config = &config;

    receivedCount = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (receivedCount != 0) {
        printf("WSAStartup failed with error: %d\n", receivedCount);
        return false;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *result = nullptr;
    receivedCount = getaddrinfo(nullptr, DEFAULT_PORT, &hints, &result);
    if (receivedCount != 0 ) {
        printf("getaddrinfo failed with error: %d\n", receivedCount);
        cleanup();
        return false;
    }

    listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (listenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        cleanup();
        return false;
    }

    unsigned long ul = 1;
    int nRet = ioctlsocket(listenSocket, FIONBIO, (unsigned long *) &ul);

    if (nRet == SOCKET_ERROR){
        printf("Put socket to async mode failed with error: %d\n", nRet);
        freeaddrinfo(result);
        cleanup();
        return false;
    }

    receivedCount = bind(listenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (receivedCount == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        cleanup();
        return false;
    }

    freeaddrinfo(result);

    receivedCount = listen(listenSocket, SOMAXCONN);
    if (receivedCount == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        cleanup();
        return false;
    }

    associatedThread = std::thread([&](){
        run();
    });
    return true;
}

bool Chatter::payload() {
    //std::cout << "chatter payload" << std::endl;
    if(clientSocket == INVALID_SOCKET){
        return waitForClient();
    }
    //std::cout << "receiving" << std::endl;
    WSASetLastError(0);
    receivedCount = 0;
    receivedCount = recv(clientSocket, recvbuf, recvbuflen, 0);
    if(WSAGetLastError() == WSAEWOULDBLOCK || receivedCount == 0){
        //std::cout << "received bad" << std::endl;
        std::chrono::duration<double> deadDuration = std::chrono::high_resolution_clock::now() - lastTime;
        //std::cout << deadDuration.count() << std::endl;
        if(deadDuration.count() > config->connectionDeadTime){
            //std::cout << "closing socket" << std::endl;
            closesocket(clientSocket);
            clientSocket = INVALID_SOCKET;
            printf("Closing connection due to zero data or WASEWOULDBLOCK\n");
            return false;
        }
        //std::cout << "sleeping" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(config->commandTimeout));
        //std::cout << "sleeped well" << std::endl;
        return false;
    }else {
        if (receivedCount > 0) {
            //std::cout << "received ok" << std::endl;
            lastTime = std::chrono::high_resolution_clock::now();
            //std::cout << "parsing..." << std::endl;
            parseCmd();
            return false;
        } else {
            //std::cout << "recv failed" << std::endl;
            printf("recv failed with error: %d\n", WSAGetLastError());
            close();
            return true;
        }
    }
}


Chatter::~Chatter() {
    requestStop();
    if(associatedThread.joinable()){
        associatedThread.join();
    }
    std::cout << "chatter closed" << std::endl;
}

void Chatter::afterPayload() {
    std::cout << "after payload cleanup" << std::endl;
    close();
    messages.putMessage(3);
}

bool Chatter::waitForClient() {
    //std::cout << "waiting for a client" << std::endl;
    clientSocket = accept(listenSocket, NULL, NULL);
    if (clientSocket == INVALID_SOCKET) {
        if(WSAGetLastError() == WSAEWOULDBLOCK){
            WSASetLastError(0);
            std::this_thread::sleep_for(std::chrono::seconds(config->connectionTimeout));
            return false;
        }
        printf("accept failed with error: %d, expected %d\n", WSAGetLastError(), WSAEWOULDBLOCK);
        close();
        return true;
    }else{
        lastTime = std::chrono::high_resolution_clock::now();
        //std::cout << "client accepted" << std::endl;
    }
    return false;
}

bool Chatter::parseCmd() {
    //printf("Bytes received: %d\n", receivedCount);
    int currentPos = 0;
    if(recvbuf[currentPos] == jsonStart){
        currentPos++;
        int cumulative = 1;
        while(cumulative != 0 && currentPos < receivedCount - 1){
            currentPos++;
            if(recvbuf[currentPos] == jsonStart){
                cumulative++;
            }else if(recvbuf[currentPos] == jsonEnd){
                cumulative--;
            }
        }
        if(recvbuf[currentPos++] == jsonEnd){
            /*
            for(int i = 0; i <= currentPos; i++){
                std::cout << recvbuf[i];
            }
            std::cout << std::endl;
             */
            if(currentPos != receivedCount){
                std::cout << "wtf: packet is only " << currentPos << "bytes" << std::endl;
            }

            if(currentPos != recvbuflen){
                recvbuf[currentPos] = 0;  //create null-terminated packet
            }
            //printf("Packet OK, end = %d\n", currentPos);

            Json packet;
            try{
                packet = Json::parse(recvbuf);
                if(packet.contains("cmd")){
                    bool found = false;
                    for(size_t cmd_index = 0; cmd_index < commands.size(); cmd_index++){
                        if(packet["cmd"] == commands[cmd_index]){
                            found = true;
                            switch (cmd_index) {
                                case 0:
                                    std::cout << "alive request" << std::endl;
                                    messages.putMessage(0);
                                    break;
                                case 1:
                                    std::cout << "arm request" << std::endl;
                                    messages.putMessage(1, packet);
                                    //add shot number and isPlasma!
                                    break;
                                case 2:
                                    std::cout << "disarm request" << std::endl;
                                    messages.putMessage(2);
                                    break;
                                case 3:
                                    std::cout << "exit request" << std::endl;
                                    messages.putMessage(3);
                                    break;
                                case 4:
                                    std::cout << "closeSocket request" << std::endl;
                                    closesocket(clientSocket);
                                    clientSocket = INVALID_SOCKET;
                                    break;
                                default:
                                    std::cout << "wtf" << std::endl;
                            }
                        }
                    }
                    if(!found){
                        std::cout << "Unknown command! \"" << packet["cmd"] << '\"' << std::endl;
                    }
                }
            }catch(Json::parse_error& err){
                std::cout << "Failed to parse json: " << err.what() << std::endl;
            }
            /*
            if(clientSocket != INVALID_SOCKET){
                sendPacket(recvbuf, currentPos);
            }
             */
            return false;
        }
    }
    std::cout << "Received not a valid JSON!" <<std::endl;
    return true;
}

void Chatter::cleanup() {
    //std::cout << "cleanup...";
    if(listenSocket != INVALID_SOCKET){
        closesocket(listenSocket);
    }
    close();
    WSACleanup();
    clientSocket = INVALID_SOCKET;
    //std::cout << "ok" << std::endl;
}

void Chatter::close(){
    //std::cout << "closing...";
    if(clientSocket != INVALID_SOCKET){
        receivedCount = shutdown(clientSocket, SD_SEND);
        if(receivedCount == WSANOTINITIALISED){
            std::cout << "ok. Already closed" << std::endl;
            return;
        }
        if (receivedCount == SOCKET_ERROR) {
            printf("shutdown failed with error: %d\n", WSAGetLastError());
            return;
        }
        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET;
    }
    //std::cout << "ok" << std::endl;
}

bool Chatter::sendPacket(Json &payload) {
    payload["cmd"] = commands[payload["id"]];
    payload.erase(payload.find("id"));
    std::string serialData = payload.dump();
    return sendPacket(serialData.c_str(), serialData.length());
}

bool Chatter::sendPacket(const char* payload, int length) {
    sendCount = send(clientSocket, payload, length, 0 );
    if (sendCount == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        close();
        return false;
    }
    close();
    return true;
}

