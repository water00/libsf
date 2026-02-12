#pragma once

#include "sfThreadBase.h"
#include <vector>

template <typename PROCESSFN>
class SFThread : public SFThreadBase <PROCESSFN>
{
private:
    int32_t maxEpollEvents;
    int32_t epollFd;
    int32_t fdCount;
    std::vector<epoll_event> events;

public:
    SFThread()
    {
        maxEpollEvents = 0;
        fdCount = 0;
        if((epollFd = epoll_create1(0)) < 0)
        {
            SFDebug::SF_print(std::string("Epoll failed: ") + strerror(errno));
        }
    }
    virtual ~SFThread()
    {
        close(epollFd);
    }

    virtual bool add_process(const typename SFThreadBase<PROCESSFN>::PSTRUCT& p)
    {
        if (p.sock < 0 || p.processObj == nullptr)
        {
            return false;
        }
        std::lock_guard<std::mutex> lock(SFThreadBase<PROCESSFN>::sfMutex.mutex);
        SFThreadBase<PROCESSFN>::processFnMap[p.sock] = p;

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
            events.resize(maxEpollEvents);
        }

        return true;
    }

    virtual bool rm_process(sock_size pID)
    {
        bool ret = true;
        std::lock_guard<std::mutex> lock(SFThreadBase<PROCESSFN>::sfMutex.mutex);
        typename std::map<int32_t, typename SFThreadBase<PROCESSFN>::PSTRUCT>::iterator mItr = SFThreadBase<PROCESSFN>::processFnMap.find(pID);

        if (mItr == SFThreadBase<PROCESSFN>::processFnMap.end())
        {
            ret = false;
        }
        else
        {
            if(epoll_ctl(epollFd, EPOLL_CTL_DEL, mItr->second.sock, nullptr))
            {
                SFDebug::SF_print(std::string("Remove Sock failed: ") + strerror(errno));
            }
            else
            {
                maxEpollEvents--;
                if (maxEpollEvents > 0)
                {
                    events.resize(maxEpollEvents);
                }
                else
                {
                    events.clear();
                }
            }
            SFThreadBase<PROCESSFN>::processFnMap.erase(pID);
        }
        return ret;
    }

    virtual int32_t wait_forEvents()
    {
        {
            std::lock_guard<std::mutex> lock(SFThreadBase<PROCESSFN>::sfMutex.mutex);
            if (SFThreadBase<PROCESSFN>::is_stopped() || maxEpollEvents <= 0) return 0;
        }
        fdCount = epoll_wait(epollFd, events.data(), maxEpollEvents, 5000);
        return fdCount;
    }

    virtual void process_fns()
    {
        {
            std::lock_guard<std::mutex> lock(SFThreadBase<PROCESSFN>::sfMutex.mutex);
            if (SFThreadBase<PROCESSFN>::is_stopped()) return;
        }
        for(int i = 0; i < fdCount; i++)
        {
            typename SFThreadBase<PROCESSFN>::PSTRUCT p;
            PROCESSFN pFn;
            {
                try
                {
                    std::lock_guard<std::mutex> lock(SFThreadBase<PROCESSFN>::sfMutex.mutex);
                    sock_size s = events[i].data.fd;
                    p = SFThreadBase<PROCESSFN>::processFnMap.at(s);
                    pFn = p.processFn;
                }
                catch(const std::exception& e)
                {
                    // 'at' can throw. Unlikely.
                    std::cerr << e.what() << std::endl;
                    continue;
                }
            }
            if (
                p.processObj &&
                !p.processObj->task_stopped() &&
                p.sock > 0 &&
                (SFThreadBase<PROCESSFN>::read_msg(p.sock) > 0) &&
                p.processObj->getNumMessages() > 0
            )
            {
                p.processObj->start_process();
                (p.processObj->*pFn)();
                p.processObj->end_process();
            }
        }
    }
};

