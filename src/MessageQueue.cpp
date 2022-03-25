//
// Created by ts_group on 13.07.2020.
//

#include "MessageQueue.h"

//debug
#include <iostream>
//debug

Message MessageQueue::getMessage() {
    //std::cout << "get message" << std::endl;
    const std::lock_guard<std::mutex> lock(mutex);
    if(messages.empty()){
        return Message{-1, Json()};
    }
    Message result = messages.front();
    messages.pop();
    return result;
}

void MessageQueue::putMessage(const int id, const Json& payload){
    const std::lock_guard<std::mutex> lock(mutex);
    messages.emplace(id, payload);
}

void MessageQueue::clear() {
    const std::lock_guard<std::mutex> lock(mutex);
    while(!messages.empty()){
        messages.pop();
    }
}
