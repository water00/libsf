#pragma once

#include "osRelated.h"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <cstring>
#include <map>
#include <vector>
#include <list>
#include <cstdint>
#include <thread>
#include <mutex>
#include <algorithm>
#include <cerrno>
#include <chrono>
#include <any>
#include "sfDebug.h"
#include "sfTask.h"
#include "sfMessages.h"

#define TIMER_DEBUG 0

struct TimerInfo
{
    uint64_t milliSec;  // milliSec timeout for this timer
    uint64_t expiryMs;  // Timer to expire at this time (from epoch)
    uint32_t timerID;
    SFTask* task;       // task to call when timer expires
    bool enabled;
    bool continuous;    // If continuous, restart timer again after expiry
    std::any userData;  // Save user data to be sent back when timer expires

    TimerInfo(uint64_t ms, uint32_t tID, SFTask* t, bool e, bool c, std::any uData) :
        milliSec(ms), timerID(tID), task(t), enabled(e), continuous(c), userData(uData)
    {
        expiryMs = get_now() + milliSec;
    }
    // Returns milliseconds from epoch in integral 
    uint64_t get_now()
    {
        std::chrono::milliseconds now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch());
        return now.count();
    }

    uint64_t ms_left()
    {
        int64_t r = expiryMs - get_now();
        if (r <= 0 || r > static_cast<int64_t>(milliSec))
        {
            // Return 1 to process immediately
            // Returning 0 will wait indefinitely
            return 1;
        }
        return r;
    }

    void reset_expiry()
    {
        expiryMs = get_now() + milliSec;
    }

    bool is_expired()
    {
        return get_now() >= expiryMs;
    }
};

struct Sorter
{
    bool operator() (TimerInfo a,TimerInfo b) 
    { 
        // Enabled ones come before disabled
        if (!a.enabled && b.enabled)
        {
            return false;
        }
        if (a.enabled && !b.enabled)
        {
            return true;
        }
        return (a.expiryMs < b.expiryMs);
    }

};

class SFTimer
{
private:
    std::vector<TimerInfo> timerInfos;
    int32_t idCnt;
    std::mutex tMutex;
    std::thread* tThread;
    Sorter timerSorter;
    bool stopThread;

public:
    SFTimer()
    {
        idCnt = 0;
        stopThread = false;
        tThread = new std::thread(&timer_thread, this);
    }
    ~SFTimer()
    {
        stopThread = true;
        tThread->join();
        delete tThread;
        timerInfos.clear();
    }

    uint32_t create_timer(uint64_t ms, SFTask* task, bool continuous, std::any uData = std::any())
    {
        if (task == nullptr)
        {
            return 0;
        }
        TimerInfo t(ms, ++idCnt, task, true, continuous, uData);

        tMutex.lock();
        timerInfos.push_back(t);

        // Sort based on expiry time
        std::sort (timerInfos.begin(), timerInfos.end(), timerSorter);
        tMutex.unlock();

        return idCnt;
    }
    // Delete is costly. Instead of creating and deleting, once created 
    // please use enable and disable, unless timer is no longer needed.
    bool delete_timer(uint32_t tID)
    {
        bool ret = false;
        tMutex.lock();
        for (auto vItr = timerInfos.begin(); vItr != timerInfos.end(); ++vItr)
        {
            if (vItr->timerID == tID)
            {
                timerInfos.erase(vItr);
                ret = true;
                break;
            }
        }
        tMutex.unlock();
        return ret;
    }

    void delete_allTimer()
    {
        tMutex.lock();
        for (auto vItr = timerInfos.begin(); vItr != timerInfos.end(); ++vItr)
        {
            timerInfos.erase(vItr);
        }
        timerInfos.clear();
        tMutex.unlock();
    }
    int32_t num_timers()
    {
        int32_t ret = 0;
        tMutex.lock();
        ret = static_cast<int32_t>(timerInfos.size());
        tMutex.unlock();
        return ret;
    }
    void print_timers()
    {
        tMutex.lock();
        for (auto vItr = timerInfos.begin(); vItr != timerInfos.end(); ++vItr)
        {
            std::stringstream ss;
            ss  << "ID: " << std::setw(4) << vItr->timerID 
                << ", MS: " << std::setw(10) << vItr->expiryMs 
                << ", Enabled: " << vItr->enabled 
                << ", Continuous: " << vItr->continuous; 
            SFDebug::SF_print(ss.str());
        }
        tMutex.unlock();
    }

    bool enable(uint32_t tID)
    {
        bool ret = false;
        tMutex.lock();
        for (auto vItr = timerInfos.begin(); vItr != timerInfos.end(); ++vItr)
        {
            if (vItr->timerID == tID)
            {
                // Reset timer expiry
                vItr->reset_expiry();
                vItr->enabled = true;
                // Re-sort
                std::sort (timerInfos.begin(), timerInfos.end(), timerSorter);
                ret = true;
                break;
            }
        }
        tMutex.unlock();
        return ret;
    }

    bool disable(int32_t tID)
    {
        bool ret = false;
        tMutex.lock();
        for (auto vItr = timerInfos.begin(); vItr != timerInfos.end(); ++vItr)
        {
            if (vItr->timerID == tID)
            {
                vItr->enabled = false;
                // Re-sort
                std::sort (timerInfos.begin(), timerInfos.end(), timerSorter);
                ret = true;
                break;
            }
        }
        tMutex.unlock();
        return ret;
    }

    static void timer_thread(void* ptr)
    {
        SFTimer* timer = static_cast<SFTimer*>(ptr);
        timer->process();
    }

    void process_timers()
    {
        tMutex.lock();
        auto vItr = timerInfos.begin();

        while (vItr != timerInfos.end() && vItr->enabled && vItr->is_expired())
        {
            // Send message to the respective task
            TimerMessage tMsg;
            tMsg.timerID = vItr->timerID;
            tMsg.userData = vItr->userData;
            if (vItr->task) vItr->task->addMessage(tMsg);

            // If continuous is set, reset expiry; else disable
            if (vItr->continuous)
            {
                #if (TIMER_DEBUG > 0)
                std::stringstream ss;
                ss << "TimerID: " << vItr->timerID << " is Continuous. Resetting";
                SFDebug::SF_print(ss.str());
                #endif

                vItr->reset_expiry();
            }
            else
            {
                #if (TIMER_DEBUG > 0)
                std::stringstream ss;
                ss << "TimerID: " << vItr->timerID << " is Not Continuous. Disabling";
                SFDebug::SF_print(ss.str());
                #endif

                vItr->enabled = false;
            }
            // Re-sort
            std::sort (timerInfos.begin(), timerInfos.end(), timerSorter);
            // Restart check with the top 
            vItr = timerInfos.begin();
        }
        tMutex.unlock();
    }

    void process()
    {
        while(!stopThread)
        {
            struct timeval tv = {0, 0};
            uint64_t msLeft = 0;

            tMutex.lock();
            auto vItr = timerInfos.begin();
            if (vItr != timerInfos.end())
            {
                msLeft = vItr->ms_left();
                tv.tv_sec = 0;
                tv.tv_usec = (long)msLeft * 1000;
            }
            tMutex.unlock();

            if (msLeft == 0)
            {
                sleep(1);
                continue;
            }

            // For Windows simply sleep for the time & process
            // Using select in Windows doesn't work as it requires atleast
            // one fd to be set.
            #if defined(_WIN32)
                Sleep((int32_t)msLeft);
                process_timers();
            #else
                switch(select(0, NULL, NULL, NULL, &tv))
                {
                case 0:
                    process_timers();
                    break;
                case -1:
                    SFDebug::SF_print(std::string("Timer Select failed, Reason: ") + strerror(errno));
                    break;
                default:
                    break;
                }
            #endif
        }
    }
};
