//
// Created by ts_group on 13.07.2020.
//

#ifndef CAEN743_MESSAGEQUEUE_H
#define CAEN743_MESSAGEQUEUE_H

#include <queue>
#include <mutex>
#include <utility>
#include "json.hpp"

using Json = nlohmann::json;

struct Message{
    const int id;
    const Json payload;

    Message(const int id, Json payload) : id(id), payload(std::move(payload)){};
};

class MessageQueue{
private:
    mutable std::mutex mutex;
    std::queue<Message> messages;
public:
    [[nodiscard]] Message getMessage();
    void putMessage(int id, const Json& payload = Json());
    void clear();
};

#endif //CAEN743_MESSAGEQUEUE_H
