#pragma once

#include <iostream>
#include <cstdint>
#include <cassert>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>
#include <map>
#include <cstring>
#include "osRelated.h"
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
class SFThreadBase
{
protected:
    typedef ProcessStruct<PROCESSFN> PSTRUCT;
    std::map<sock_size, PSTRUCT > processFnMap;
    std::thread iThread;
    SFMutex sfMutex;
    std::atomic_bool stopThread = false;
    bool stopped;

public:
    SFThreadBase() 
    {
        stopped = false;
        iThread = std::thread(start_thread, (void*)this);
    }

    virtual ~SFThreadBase()
    {
    }

    void stop_thread()
    {
        stopThread = true;
        iThread.join();
    }

    bool is_stopped()
    {
        return stopped;
    }


    virtual bool add_process(const PSTRUCT& p) = 0;
    virtual bool rm_process(sock_size pID) = 0;
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
                sfMutex.wait_forProcessStart();
                if (stopThread) break;
                int32_t count = wait_forEvents();
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
                sfMutex.end_process();
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


