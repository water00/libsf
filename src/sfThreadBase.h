#pragma once

#include <iostream>
#include <cstdint>
#include <cassert>
#include <thread>
#include <mutex>
#include <list>
#include <map>
#include <cstring>
#include "osRelated.h"
#include "sfDebug.h"

class SFTask;

template <typename PROCESSFN>
struct ProcessStruct
{
    sock_size sock;
    SFTask* processObj;
    PROCESSFN processFn;
};



template <typename PROCESSFN>
class SFThreadBase
{
protected:
    typedef ProcessStruct<PROCESSFN> PSTRUCT;
    std::map<sock_size, PSTRUCT > processFnMap;
    std::thread iThread;
    std::recursive_mutex iMutex;
    bool stopThread;
    bool stopped;
    int32_t fdCount;

public:
    SFThreadBase() 
    {
        stopped = false;
        stopThread = false;
        fdCount = 0;
        iThread = std::thread(start_thread, (void*)this);
    }

    virtual ~SFThreadBase()
    {
        stop_thread();
        iThread.join();
    }

    void stop_thread()
    {
        stopThread = true;
    }

    bool is_stopped()
    {
        return stopped;
    }

    inline void lock() { std::lock_guard<std::recursive_mutex> lock(iMutex); }

    virtual bool add_process(const PSTRUCT& p) = 0;
    virtual bool rm_process(int pID) = 0;
    virtual int32_t wait_forEvents() = 0;
    virtual void process_fns() = 0;
    
    static void* start_thread(void* data)
    {
        return static_cast<SFThread<PROCESSFN> *>(data)->thread_run();
    }

    void* thread_run()
    {
        stopThread = false;
        while (!stopThread)
        {
            {
                lock();
                fdCount = wait_forEvents();
                switch (fdCount)
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
            }
        }

        //SFDebug::SF_print(std::string("Thread stopped: ") + pthread_self());
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


