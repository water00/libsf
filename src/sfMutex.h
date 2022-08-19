#pragma once

#include "osRelated.h"
#include <iostream>
#include <cstdint>
#include <mutex>
#include <condition_variable>
#include "sfDebug.h"


class SFMutex
{
protected:
    std::mutex mutex;
    std::condition_variable condVar;
    std::atomic_bool inProcess;
    std::atomic_bool startProcess;

public:
    SFMutex() 
    {
        inProcess = false;
        startProcess = true;
    }

    virtual ~SFMutex()
    {
    }

    inline void lock() { std::lock_guard<std::mutex> lock(mutex); }

    inline void wait_forProcessEnd()
    {
        std::unique_lock<std::mutex> lk(mutex);
        while(inProcess)
        {
            condVar.wait(lk);
        }
        startProcess = false;
    }

    inline void restart_process()
    {
        std::unique_lock<std::mutex> lk(mutex);
        startProcess = true;
        condVar.notify_one();
    }

    inline void wait_forProcessStart()
    {
        std::unique_lock<std::mutex> lk(mutex);
        while(!startProcess)
        {
            condVar.wait(lk);
        }
        inProcess = true;
    }

    inline void end_process()
    {
        std::unique_lock<std::mutex> lk(mutex);
        inProcess = false;
        condVar.notify_all();
    }
};


