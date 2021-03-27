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
    int32_t sock;
    SFTask* processObj;
    PROCESSFN processFn;
};



template <typename PROCESSFN>
class SFThread
{
private:
    typedef ProcessStruct<PROCESSFN> PSTRUCT;
    std::map<int32_t, PSTRUCT > processFnMap;
    std::thread iThread;
    std::recursive_mutex iMutex;
    bool stopThread;
    bool stopped;

    int nfds;
    fd_set readfds;

public:
    SFThread() 
    {
        stopped = false;
        stopThread = false;
        nfds = 0;
        FD_ZERO(&readfds);
        iThread = std::thread(start_thread, (void*)this);
    }

    virtual ~SFThread()
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

    void lock()
    {
        iMutex.lock();
    }
    void unlock()
    {
        iMutex.unlock();
    }

    bool add_process(const PSTRUCT& p)
    {
        if (p.sock < 0 || p.processObj == NULL)
        {
            return false;
        }
        lock();
        processFnMap[p.sock] = p;
        unlock();

        return true;
    }

    bool rm_process(int pID)
    { 
        bool ret = true;
        int s = 0;
        lock();
        typename std::map<int32_t, PSTRUCT>::iterator mItr = processFnMap.find(pID);
 
        if (mItr == processFnMap.end())
        {
            ret = false;
        }
        else
        {
            processFnMap.erase(pID);
        }
        unlock();
        return ret;
    }

    void process_fns()
    {
        lock();
        for (typename std::map<int32_t, PSTRUCT >::iterator mItr = processFnMap.begin(); mItr != processFnMap.end(); ++mItr)
        {
            if (FD_ISSET(mItr->second.sock, &readfds))
            {
                PSTRUCT p = mItr->second;
                read_msg(p.sock);
                PROCESSFN pFn = p.processFn;
                (p.processObj->*pFn)();
            }
        }
        unlock();
    }

    void add_socks()
    {
        lock();
        nfds = 0;
        FD_ZERO(&readfds);
        for (typename std::map<int32_t, PSTRUCT>::iterator mItr = processFnMap.begin(); mItr != processFnMap.end(); ++mItr)
        {
            nfds = std::max<int32_t>(nfds,  mItr->second.sock);
            FD_SET(mItr->second.sock, &readfds);
        }
        unlock();
    }

    static void* start_thread(void* data)
    {
        return static_cast<SFThread<PROCESSFN> *>(data)->thread_run();
    }

    void* thread_run()
    {
        int fdCount = 0;
        struct timespec ts;
        ts.tv_sec = 5;
        ts.tv_nsec = 0;

        stopThread = false;
        while (!stopThread)
        {
            add_socks();

            if (nfds <= 0)
            {
                sleep(1);
                continue;
            }
            fdCount = pselect(nfds+1, &readfds, NULL, NULL, &ts, NULL);
            switch (fdCount)
            {
            case 0:
                break;
            case -1:
                SFDebug::SF_print(std::string("pselect error, Reason: ") + strerror(errno));
                break;
            default:
                // Check whether any sock has data and call the process fn accordingly
                process_fns();
                break;
            }
        }

        //SFDebug::SF_print(std::string("Thread stopped: ") + pthread_self());
        stopped = true;
        return NULL;
    }

    int32_t read_msg(int32_t sock)
    {
        // Read the data and throw away as it is just dummy msg
        char dummy[3];
        int32_t ret = 0;

        if (read(sock, dummy, sizeof(dummy)) < 0)
        {
            SFDebug::SF_print(std::string("Read failed, Reason: ") + strerror(errno));
            return -1;
        }
        return ret;
    }
};


