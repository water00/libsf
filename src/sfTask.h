#pragma once

#include "sfThread.h"
#include "sfMessages.h"
#include <memory>
#if defined(_WIN32)
#include "winSockPair.h"
#endif

typedef void(SFTask::*PROCFN)(void);

class SFTask
{   
private:
   	std::list<std::shared_ptr<SFMessage> > messages;
    std::mutex processMutex;

protected:

    int32_t socks[2];
    static int32_t taskCount;

public:
	static SFThread<PROCFN> *sfThread;

    SFTask()
    {
        socks[0] = -1;
        socks[1] = -1;
        
        if (sfThread == NULL)
        {
            sfThread = new SFThread<PROCFN> ();
        }
        taskCount++;

    }
    virtual ~SFTask()
    {
        for (auto lItr = messages.begin(); lItr != messages.end(); ++lItr)
        {
            lItr->reset();
        }
        messages.clear();
        if (--taskCount == 0)
        {
            delete sfThread;
        }
    }
   	virtual void processFn()  = 0;

    virtual void shut_down()
    {
        processMutex.lock();
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
        processMutex.unlock();
    }
    template <typename T>
    bool addMessage(const T& msg)
    {
        bool ret = false;
        if (socks[0] > 0 && socks[1] > 0)
        {
            processMutex.lock();
            messages.push_back(std::make_shared<T>(msg));
            processMutex.unlock();
            // Indicate(send) that message is ready
            ret = send_msg(socks[1]) > 0 ? true : false;
        }
        return ret;
    } 

    template <typename T>
    bool getMessage(T& msg)
    {
        bool ret = false;
        processMutex.lock();
        if (!messages.empty())
        {
            msg = *(dynamic_cast<T*>(messages.front().get()));
            messages.pop_front();
            ret = true;
        }
        processMutex.unlock();

        return ret;
    }

    int32_t getCommand()
    {
        int32_t cmd = -1;
        processMutex.lock();        
        if (!messages.empty())
        {
            cmd = messages.front().get()->command;
        }
        processMutex.unlock();

        return cmd;
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

    int32_t send_msg(int32_t sock)
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

