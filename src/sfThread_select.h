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
class SFThread
{
private:
    typedef ProcessStruct<PROCESSFN> PSTRUCT;
    std::map<sock_size, PSTRUCT > processFnMap;
    std::thread iThread;
    std::recursive_mutex iMutex;
    bool stopThread;
    bool stopped;

    sock_size nfds;
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
        typename std::map<sock_size, PSTRUCT>::iterator mItr = processFnMap.find(pID);
 
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
        for (typename std::map<sock_size, PSTRUCT >::iterator mItr = processFnMap.begin(); mItr != processFnMap.end(); ++mItr)
        {
            PSTRUCT p = mItr->second;
            if (FD_ISSET(p.sock, &readfds) && (read_msg(p.sock) > 0))
            {
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
        for (typename std::map<sock_size, PSTRUCT>::iterator mItr = processFnMap.begin(); mItr != processFnMap.end(); ++mItr)
        {
            nfds = std::max<sock_size>(nfds,  mItr->second.sock);
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
        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        stopThread = false;
        while (!stopThread)
        {
            add_socks();

            if (nfds <= 0)
            {
                sleep(1);
                continue;
            }
            fdCount = select((int32_t)nfds+1, &readfds, NULL, NULL, &tv);
            switch (fdCount)
            {
            case 0:
                break;
            case -1:
                SFDebug::SF_print(std::string("select error, Reason: ") + strerror(errno));
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


