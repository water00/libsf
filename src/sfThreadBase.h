#pragma once

#include "osRelated.h"
#include <iostream>
#include <cstdint>
#include <cassert>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>
#include <map>
#include <cstring>
#include "sfDebug.h"
#include "sfMutex.h"

class SFTask;

template <typename PROCESSFN>
struct ProcessStruct
{
    sock_size sock;
    SFTask* processObj;
    PROCESSFN processFn;
};

template <typename PROCESSFN>
class SFThread;


template <typename PROCESSFN>
class SFThreadBase
{
protected:
    typedef ProcessStruct<PROCESSFN> PSTRUCT;
    std::map<sock_size, PSTRUCT > processFnMap;
    std::thread* iThread;
    SFMutex sfMutex;
    std::atomic_bool stopThread;
    std::atomic_bool stopped;

public:
    SFThreadBase() 
    {
        stopThread = false;
        stopped = false;
        iThread = new std::thread(&SFThreadBase::start_thread, this);
    }

    virtual ~SFThreadBase()
    {
        SFDebug::SF_print(__FUNCTION__);
        sfMutex.stop();
        stopThread = true;
        sfMutex.notify();
        while (!stopped) sleep(1);
        iThread->join();
        delete iThread;
    }

    virtual void stop_thread()
    {
        stopThread = true;
    }
    bool is_stopped()
    {
        return stopThread || stopped;
    }

    int64_t get_processCount()
    {
        std::lock_guard<std::mutex> lock(sfMutex.mutex);
        return processFnMap.size();
    }
    virtual void wait_forProcessEnd() 
    { 
        sfMutex.wait_forProcessEnd(); 
    }
    virtual void restart_process() 
    { 
        sfMutex.restart_process(); 
    }
    virtual void stop_process()
    {
        sfMutex.stop_process();
    }

    virtual bool add_process(const PSTRUCT& p) = 0;
    virtual bool rm_process(sock_size pID) = 0;
    virtual int32_t wait_forEvents() = 0;
    virtual void process_fns() = 0;
    
    static void* start_thread(void* data)
    {
        return (static_cast<SFThread<PROCESSFN> *>(data))->thread_run();
    }

    void* thread_run()
    {
        stopThread = false;
        while (!stopThread)
        {
            sfMutex.wait_forProcessStart();
            if (stopThread) break;
            int32_t count = wait_forEvents();
            if (stopThread) break;
            switch (count)
            {
            case 0:
                break;
            case -1:
                SFDebug::SF_print(std::string("SFThread error, Reason: ") + strerror(errno));
                break;
            default:
                // Check whether any sock has data and call the process fn accordingly
                process_fns();
                break;
            }
            if (stopThread) break;
            sfMutex.end_process();
        }

        stopped = true;
        return NULL;
    }

    int32_t read_msg(sock_size sock)
    {
        // Read the data and throw away as it is just dummy msg
        char dummy[3];
        int32_t ret = 0;

        if ((ret = recv(sock, dummy, sizeof(dummy), 0)) < 0)
        {
            SFDebug::SF_print(std::string("Read failed, Reason: ") + strerror(errno));
        }
        return ret;
    }
};


