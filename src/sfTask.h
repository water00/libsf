#pragma once

#include "sfThread.h"
#include "sfMessages.h"
#include "sfMutex.h"
#include <memory>
#if defined(_WIN32)
#include "winSockPair.h"
// To check memory leak
#ifdef VLD
#include <vld.h>
#endif
#endif

typedef void(SFTask::*PROCFN)(void);

class SFTask
{   
private:
    std::list<std::shared_ptr<SFMessage> > messages;
protected:

    sock_size socks[2];
    inline static int32_t taskCount = 0;
    bool stopTask;
    SFMutex sfMutex;

public:
    inline static SFThread<PROCFN> *sfThread = nullptr;

    friend SFThreadBase;

    SFTask()
    {
        socks[0] = -1;
        socks[1] = -1;
        stopTask = false;

        if (sfThread == nullptr)
        {
            sfThread = new SFThread<PROCFN> ();
        }
        taskCount++;
    }
    virtual ~SFTask()
    {
        // If task processing is going on, wait
        sfMutex.wait_forProcessEnd();
        stopTask = true;
        sfThread->rm_process(socks[0]);
        shut_down();
        del_msgs();
        if (--taskCount == 0)
        {
            sfThread->stop_thread();
            delete sfThread;
        }
    }
    // Call do_endProcess before deleting the task
    // so that residual messages are processed.
    virtual void do_endProcess()
    {
        while(getNumMessages())
        {
            sleep(1);
        }
        sfMutex.wait_forProcessEnd();
    }
    void del_msgs()
    {
        sfMutex.lock();
        for (auto lItr = messages.begin(); lItr != messages.end(); ++lItr)
        {
            lItr->reset();
        }
        messages.clear();

    }
    bool task_stopped()
    {
        return stopTask;
    }
    void start_process()
    {
        sfMutex.wait_forProcessStart();
    }
    void end_process()
    {
        sfMutex.end_process();
    }
    virtual void wait_forProcessEnd() 
    { 
        sfMutex.wait_forProcessEnd(); 
    }
    virtual void restart_process() 
    { 
        sfMutex.restart_process(); 
    }

    virtual void processFn()  = 0;

    virtual void shut_down()
    {
        sfMutex.lock();
        if (socks[0] > 0 && socks[1] > 0)
        {
            #ifdef _WIN32
                shutdown(socks[0], SD_BOTH);
                shutdown(socks[1], SD_BOTH);
            #else
                shutdown(socks[0], SHUT_RDWR);
                shutdown(socks[1], SHUT_RDWR);
            #endif
            socks[0] = socks[1] = -1;
        }
    }
    template <typename T>
    bool addMessage(const T& msg)
    {
        bool ret = false;
        sfMutex.lock();
        if (socks[0] > 0 && socks[1] > 0)
        {
            messages.push_back(std::make_shared<T>(msg));
            // Indicate(send) that message is ready
            ret = send_msg(socks[1]) > 0 ? true : false;
        }
        return ret;
    } 

    template <typename T>
    bool getMessage(T& msg)
    {
        bool ret = false;
        sfMutex.lock();
        if (!messages.empty())
        {
            msg = *(dynamic_cast<T*>(messages.front().get()));
            messages.pop_front();
            ret = true;
        }

        return ret;
    }

    int32_t getNumMessages()
    {
        int32_t ret = 0;
        sfMutex.lock();
        ret = (int32_t)messages.size();

        return ret;
    }

    template <typename T>
    bool peekMessage(T& msg)
    {
        bool ret = false;
        sfMutex.lock();
        if (!messages.empty())
        {
            msg = *(dynamic_cast<T*>(messages.front().get()));
            ret = true;
        }
        return ret;
    }

    bool getMessageType(SFType& type)
    {
        bool ret = false;
        sfMutex.lock();
        if (!messages.empty())
        {
            type = messages.front()->getType();
            ret = true;
        }
        return ret;
    }

    void create_sock()
    {
        #if defined(_WIN32)
            if (!WinSockPair::get_sockPair(socks))
            {
                SFDebug::SF_print(std::string("SocketPair Creation failed"));
            }
        #else
            if ((socketpair(AF_UNIX, SOCK_STREAM, 0, socks)) < 0)
            {
                SFDebug::SF_print(std::string("Socket Creation failed, Reason: ") + strerror(errno));
            }
            // We can put socket options here, though at present we don't need any.
        #endif
    }

    int32_t send_msg(sock_size sock)
    {
        char dummy[] = "\n\r";
        int32_t ret = 0;

        if ((ret = send(sock, dummy, (int32_t)strlen(dummy), 0)) < 0)
        {
            SFDebug::SF_print(std::string("Write failed, Reason: ") + strerror(errno));
            ret = -1;
        }
        return ret;
    }

};

