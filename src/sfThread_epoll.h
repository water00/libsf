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

    int maxEpollEvents;
    int epollFd;

public:
    SFThread() 
    {
        stopped = false;
        stopThread = false;
        maxEpollEvents = 0;
        if((epollFd = epoll_create1(0)) < 0)
        {
            SFDebug::SF_print(std::string("Epoll failed: ") + strerror(errno));
        }
        iThread = std::thread(start_thread, (void*)this);
    }

    virtual ~SFThread()
    {
        stop_thread();
        close(epollFd);
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

        struct epoll_event event;

        event.events = EPOLLIN;
        event.data.ptr = nullptr;
        event.data.fd = p.sock;
        
        if(epoll_ctl(epollFd, EPOLL_CTL_ADD, p.sock, &event) < 0)
        {
            SFDebug::SF_print(std::string("Add Sock failed: ") + strerror(errno));
        }
        else
        {
            maxEpollEvents++;
        }
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
            if(epoll_ctl(epollFd, EPOLL_CTL_DEL, mItr->second.sock, NULL))
            {
                SFDebug::SF_print(std::string("Remove Sock failed: ") + strerror(errno));
            }
            else
            {
                maxEpollEvents--;
            }
            processFnMap.erase(pID);
        }
        unlock();
        return ret;
    }

    void process_fns(int32_t s)
    {
        lock();
        PSTRUCT p = processFnMap[s];
        if (read_msg(p.sock) > 0)
        {
            PROCESSFN pFn = p.processFn;
            (p.processObj->*pFn)();
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

        stopThread = false;
        while (!stopThread)
        {
            struct epoll_event* events = new epoll_event[maxEpollEvents];
            fdCount = epoll_wait(epollFd, events, maxEpollEvents, 5000);
            switch (fdCount)
            {
            case 0:
                break;
            case -1:
                SFDebug::SF_print(std::string("ePoll error, Reason: ") + strerror(errno));
                break;
            default:
                for(int i = 0; i < fdCount; i++)
                {
                    process_fns(events[i].data.fd);
                }
                break;
            }
            delete [] events;
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

        if ((ret = read(sock, dummy, sizeof(dummy))) < 0)
        {
            SFDebug::SF_print(std::string("Read failed, Reason: ") + strerror(errno));
        }
        return ret;
    }
};


