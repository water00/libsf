#pragma once

#include "sfThreadBase.h"

template <typename PROCESSFN>
class SFThread : public SFThreadBase
{
private:
    int maxEpollEvents;
    int epollFd;
    struct epoll_event* events;
    
public:
    SFThread() 
    {
        maxEpollEvents = 0;
        if((epollFd = epoll_create1(0)) < 0)
        {
            SFDebug::SF_print(std::string("Epoll failed: ") + strerror(errno));
        }
    }
    virtual ~SFThread()
    {
    }

    virtual ~SFThread()
    {
        close(epollFd);
        if (events)
        {
            delete [] events;
        }
    }

    virtual bool add_process(const PSTRUCT& p)
    {
        if (p.sock < 0 || p.processObj == NULL)
        {
            return false;
        }
        sfMutex.lock();
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
            // Delete previous events and create new
            if (events)
            {
                delete [] events;
            }
            events = new epoll_event[maxEpollEvents];
        }

        return true;
    }

    virtual bool rm_process(sock_size pID)
    { 
        bool ret = true;
        sfMutex.lock();
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
                // Delete previous events and create new
                if (events)
                {
                    delete [] events;
                }
                events = new epoll_event[maxEpollEvents];
            }
            processFnMap.erase(pID);
        }
        return ret;
    }

    virtual int32_t wait_forEvents()
    {
        return epoll_wait(epollFd, events, maxEpollEvents, 5000);
    }
    
    virtual void process_fns()
    {
        for(int i = 0; i < fdCount; i++)
        {
            sock_size s = events[i].data.fd;
            PSTRUCT p = processFnMap[s];
            if (read_msg(p.sock) > 0)
            {
                PROCESSFN pFn = p.processFn;
                if (p.processObj && !p.processObj->task_stopped())
                {
                    p.processObj->start_process();
                    (p.processObj->*pFn)();
                    p.processObj->end_process();
                }
            }
        }
    }

};


