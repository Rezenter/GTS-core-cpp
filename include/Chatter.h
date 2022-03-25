//
// Created by ts_group on 10.07.2020.
//

#ifndef CAEN743_CHATTER_H
#define CAEN743_CHATTER_H

#ifndef UNICODE
#define UNICODE 1
#endif

#include "Stoppable.h"
#include "Config.h"
#include "MessageQueue.h"
#include "json.hpp"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <chrono>
#include <vector>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

using Json = nlohmann::json;


class Chatter : public Stoppable{
private:
    std::vector<std::string> commands = {
            "alive",
            "arm",
            "disarm",
            "exit",
            "close"
    };
    /*
    std::map<std::string, int> commands = {
            {"alive", 0},
            {"arm", 1 },
            {"disarm", 2},
            {"exit", 3},
            {"close", 4},
    };
*/
    const char jsonStart = '{';
    const char jsonEnd = '}';

    Config* config;

    std::chrono::time_point<std::chrono::high_resolution_clock> lastTime = std::chrono::high_resolution_clock::now();

    WSADATA wsaData;
    int receivedCount;

    SOCKET listenSocket = INVALID_SOCKET;
    SOCKET clientSocket = INVALID_SOCKET;

    struct addrinfo hints;

    int sendCount;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    bool waitForClient();
    bool parseCmd();
    void cleanup();
    void close();

    bool payload() override;
    //void beforePayload() override;
    void afterPayload() override;
public:
    MessageQueue messages;

    ~Chatter();
    bool init(Config& config);

    bool sendPacket(Json& payload);
    bool sendPacket(const char* payload, int length);
};


#endif //CAEN743_CHATTER_H
