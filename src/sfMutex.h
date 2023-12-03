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
    static std::mutex mutex;
    std::mutex condMutex;
    std::condition_variable startCondVar;
    std::condition_variable endCondVar;
    std::atomic_bool inProcess = false;
    std::atomic_bool startProcess = true;
    std::atomic_bool stopped = false;

public:
    SFMutex() 
    {
    }

    virtual ~SFMutex()
    {
    }

    inline void lock() { std::lock_guard<std::mutex> lock(mutex); }

    inline void wait_forProcessEnd()
    {
        std::unique_lock<std::mutex> lk(condMutex);
        while(inProcess)
        {
            if (stopped)
            {
                return;
            }
            endCondVar.wait(lk);
        }
    }

    inline void stop_process()
    {
        std::unique_lock<std::mutex> lk(condMutex);
        while (inProcess)
        {
            if (stopped)
            {
                return;
            }
            endCondVar.wait(lk);
        }
        startProcess = false;
    }
    inline void restart_process()
    {
        std::unique_lock<std::mutex> lk(condMutex);
        startProcess = true;
        startCondVar.notify_all();
    }

    inline void wait_forProcessStart()
    {
        std::unique_lock<std::mutex> lk(condMutex);
        while(!startProcess)
        {
            if (stopped)
            {
                return;
            }
            startCondVar.wait(lk);
        }
    }

    inline void end_process()
    {
        std::unique_lock<std::mutex> lk(condMutex);
        inProcess = false;
        endCondVar.notify_all();
    }

    inline void stop()
    {
        stopped = true;
    }

    inline void notify()
    {
        startCondVar.notify_all();
        endCondVar.notify_all();
    }
};
