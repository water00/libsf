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
        std::cout << __FUNCTION__ << std::endl;
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
        bool ret = true;
        sfMutex.lock();
        typename std::map<sock_size, PSTRUCT>::iterator mItr = processFnMap.find(pID);
 
        if (mItr == processFnMap.end())
        {
            ret = false;
        }
        else
        {
            processFnMap.erase(pID);
        }
        return ret;
    }

    virtual int32_t wait_forEvents()
    {
        nfds = 0;
        fdCount = 0;
        FD_ZERO(&readfds);
        for (typename std::map<sock_size, PSTRUCT>::iterator mItr = processFnMap.begin(); mItr != processFnMap.end(); ++mItr)
        {
            nfds = std::max<sock_size>(nfds,  mItr->second.sock);
            FD_SET(mItr->second.sock, &readfds);
        }
        if (nfds > 0)
        {
            fdCount = select((int32_t)nfds+1, &readfds, NULL, NULL, &tv);
        }
        return fdCount;
    }

    virtual void process_fns()
    {
        for (typename std::map<sock_size, PSTRUCT >::iterator mItr = processFnMap.begin(); mItr != processFnMap.end(); ++mItr)
        {
            PSTRUCT p = mItr->second;
            if (p.sock <= 0)
                continue;
            if (FD_ISSET(p.sock, &readfds) && (read_msg(p.sock) > 0))
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


