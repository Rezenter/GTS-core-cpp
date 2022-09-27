//
// Created by ts_group on 6/28/2020.
//

#ifndef CAEN743_STOPPABLE_H
#define CAEN743_STOPPABLE_H

#include <thread>
#include <chrono>
#include <mutex>

#include <iostream>

class Stoppable{
private:
    mutable std::mutex mutex;

protected:
    virtual bool payload() = 0; //this one is pure virtual. return isFinished
    virtual void beforePayload(){};
    virtual void afterPayload(){};
    bool stop = false;

public:
    std::thread associatedThread;
    virtual ~Stoppable(){
        requestStop();
        associatedThread.join();
    };

    void requestStop(){
        mutex.lock();
        stop = true;
        mutex.unlock();
    };

    void run(){
        mutex.lock();
        stop = false;
        mutex.unlock();

        beforePayload();
        while(true){
            mutex.lock();
            if(stop){
                mutex.unlock();
                break;
            }
            mutex.unlock();
            if(payload()){
                break;
            }
        }
        afterPayload();
    };
};

#endif //CAEN743_STOPPABLE_H
