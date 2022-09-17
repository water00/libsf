#pragma once

#include "SFThreadBase.h"

template <typename PROCESSFN>
class SFThread : public SFThreadBase <PROCESSFN>
{
private:
    sock_size nfds;
    fd_set readfds;
    struct timeval tv;

public:
    SFThread() 
    {
        nfds = 0;
        FD_ZERO(&readfds);
        // Setup timer for select.
        tv.tv_sec = 5;
        tv.tv_usec = 0;
    }
    virtual ~SFThread()
    {
    }

    virtual bool add_process(const PSTRUCT& p)
    {
        if (p.sock < 0 || p.processObj == NULL)
        {
            return false;
        }
        sfMutex.lock();
        processFnMap[p.sock] = p;

        return true;
    }

    virtual bool rm_process(sock_size pID)
    { 
        bool ret = false;
        sfMutex.lock();
        typename std::map<sock_size, PSTRUCT>::iterator mItr = processFnMap.find(pID);
 
        if (mItr != processFnMap.end())
        {
            processFnMap.erase(pID);
            ret = true;
        }
        return ret;
    }

    virtual int32_t wait_forEvents()
    {
        sfMutex.lock();
        nfds = 0;
        FD_ZERO(&readfds);
        if (is_stopped() || !processFnMap.size()) return 0;
        for (typename std::map<sock_size, PSTRUCT>::iterator mItr = processFnMap.begin(); mItr != processFnMap.end(); ++mItr)
        {
            sock_size s = mItr->second.sock;
            if (s > 0)
            {
                nfds = std::max<sock_size>(nfds, s);
                FD_SET(s, &readfds);
            }
        }
        if (nfds > 0)
        {
            return select((int32_t)nfds+1, &readfds, NULL, NULL, &tv);
        }
        return 0;
    }

    virtual void process_fns()
    {
        sfMutex.lock();
        if (is_stopped() || !processFnMap.size()) return;
        for (typename std::map<sock_size, PSTRUCT >::iterator mItr = processFnMap.begin(); mItr != processFnMap.end(); ++mItr)
        {
            PSTRUCT p = mItr->second;
            PROCESSFN pFn = p.processFn;
            if (
                p.processObj && 
                !p.processObj->task_stopped() && 
                p.sock > 0 &&
                FD_ISSET(p.sock, &readfds) && 
                (read_msg(p.sock) > 0) && 
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


