#pragma once

#include "sfThreadBase.h"

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
        tv.tv_sec = 0;
        tv.tv_usec = 0;
    }
    virtual ~SFThread()
    {
    }

    virtual bool add_process(const typename SFThreadBase<PROCESSFN>::PSTRUCT& p)
    {
        if (p.sock < 0 || p.processObj == NULL)
        {
            return false;
        }
        std::lock_guard<std::mutex> lock(SFThreadBase<PROCESSFN>::sfMutex.mutex);
        SFThreadBase<PROCESSFN>::processFnMap[p.sock] = p;

        return true;
    }

    virtual bool rm_process(sock_size pID)
    { 
        bool ret = false;
        std::lock_guard<std::mutex> lock(SFThreadBase<PROCESSFN>::sfMutex.mutex);
        typename std::map<sock_size, typename SFThreadBase<PROCESSFN>::PSTRUCT>::iterator mItr = SFThreadBase<PROCESSFN>::processFnMap.find(pID);
 
        if (mItr != SFThreadBase<PROCESSFN>::processFnMap.end())
        {
            SFThreadBase<PROCESSFN>::processFnMap.erase(pID);
            ret = true;
        }
        return ret;
    }

    virtual int32_t wait_forEvents()
    {
        nfds = 0;
        FD_ZERO(&readfds);
        tv.tv_sec = 0;
        tv.tv_usec = 0;

        typename std::map<sock_size, typename SFThreadBase<PROCESSFN>::PSTRUCT>::iterator mItr;
        {
            std::lock_guard<std::mutex> lock(SFThreadBase<PROCESSFN>::sfMutex.mutex);
            if (SFThreadBase<PROCESSFN>::is_stopped() || !SFThreadBase<PROCESSFN>::processFnMap.size()) return 0;
            mItr = SFThreadBase<PROCESSFN>::processFnMap.begin();
        }

        do {
            sock_size s;
            {
                std::lock_guard<std::mutex> lock(SFThreadBase<PROCESSFN>::sfMutex.mutex);
                if (SFThreadBase<PROCESSFN>::is_stopped()) return 0;
                if (mItr == SFThreadBase<PROCESSFN>::processFnMap.end()) break;
                s = mItr->second.sock;
                mItr++;
            }
            if (s > 0)
            {
                nfds = std::max<sock_size>(nfds, s);
                FD_SET(static_cast<uint64_t>(s), &readfds);
            }
        } while (1);

        if (nfds > 0)
        {
            return select((int32_t)nfds+1, &readfds, NULL, NULL, &tv);
        }
        return 0;
    }

    virtual void process_fns()
    {
        typename std::map<sock_size, typename SFThreadBase<PROCESSFN>::PSTRUCT>::iterator mItr;
        {
            std::lock_guard<std::mutex> lock(SFThreadBase<PROCESSFN>::sfMutex.mutex);
            if (SFThreadBase<PROCESSFN>::is_stopped() || !SFThreadBase<PROCESSFN>::processFnMap.size()) return;
            mItr = SFThreadBase<PROCESSFN>::processFnMap.begin();
        }

        do {
            typename SFThreadBase<PROCESSFN>::PSTRUCT p;
            PROCESSFN pFn;
            {
                std::lock_guard<std::mutex> lock(SFThreadBase<PROCESSFN>::sfMutex.mutex);
                if (SFThreadBase<PROCESSFN>::is_stopped()) return;
                if (mItr == SFThreadBase<PROCESSFN>::processFnMap.end()) return;
                p = mItr->second;
                pFn = p.processFn;
                mItr++;
            }
            if (
                p.processObj && 
                !p.processObj->task_stopped() && 
                p.sock > 0 &&
                FD_ISSET(p.sock, &readfds) && 
                (SFThreadBase<PROCESSFN>::read_msg(p.sock) > 0) && 
                p.processObj->getNumMessages() > 0 
            )
            {
                p.processObj->start_process();
                (p.processObj->*pFn)();
                p.processObj->end_process();
            }
        } while (1);
    }
};


