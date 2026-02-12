#pragma once

#include "osRelated.h"
#include "sfThread.h"
#include "sfMessages.h"
#include "sfMutex.h"
#include <memory>
#include <mutex>

typedef void(SFTask::*PROCFN)(void);

class SFTask
{
private:
    std::list<std::shared_ptr<SFMessage> > messages;
public:
    sock_size socks[2];
protected:
    std::atomic_bool stopTask;
    SFMutex sfMutex;

public:
    inline static SFThread<PROCFN> *sfThread = nullptr;
    inline static std::mutex sfThreadMutex;

    SFTask()
    {
        socks[0] = -1;
        socks[1] = -1;
        stopTask = false;

        {
            std::lock_guard<std::mutex> lock(sfThreadMutex);
            if (sfThread == nullptr)
            {
                sfThread = new SFThread<PROCFN>();
            }
        }
    }
    virtual ~SFTask()
    {
        SFDebug::SF_print(__FUNCTION__);
        // If task processing is going on, wait
        sfMutex.wait_forProcessEnd();
        std::lock_guard<std::mutex> lock(sfMutex.mutex);
        stopTask = true;
        //del_msgs();
        for (auto lItr = messages.begin(); lItr != messages.end(); ++lItr)
        {
            lItr->reset();
        }
        messages.clear();
        if (sfThread)
        {
            sfThread->stop_process();
            sfThread->rm_process(socks[0]);
            //sfThread->restart_process();
            shut_down();
            int64_t count;
            if ((count = sfThread->get_processCount()) == 0)
            {
                sleep(1);
                delete sfThread;
                sfThread = nullptr;
            }
            else
            {
                //std::stringstream ss;
                //ss << "Process Count: " << count;
                //SFDebug::SF_print(ss.str());
            }
        }
        if (sfThread)
        {
            sfThread->restart_process();
        }
    }
    // Call do_endProcess before deleting the task
    // so that residual messages are processed.
    virtual void do_endProcess()
    {
	/*
        while(getNumMessages())
        {
            sleep(1);
        }
	*/
        sfMutex.wait_forProcessEnd();
    }
    void del_msgs()
    {
        std::lock_guard<std::mutex> lock(sfMutex.mutex);
        for (auto lItr = messages.begin(); lItr != messages.end(); ++lItr)
        {
            lItr->reset();
        }
        messages.clear();

    }
    void stop_task()
    {
        std::lock_guard<std::mutex> lock(sfMutex.mutex);
        stopTask = true;
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
        if (socks[0] > 0 && socks[1] > 0)
        {
            #ifdef _WIN32
                shutdown(socks[0], SD_BOTH);
                shutdown(socks[1], SD_BOTH);
                closesocket(socks[0]);
                closesocket(socks[1]);
            #else
                shutdown(socks[0], SHUT_RDWR);
                shutdown(socks[1], SHUT_RDWR);
                close(socks[0]);
                close(socks[1]);
            #endif
            socks[0] = socks[1] = -1;
        }
    }
    template <typename T>
    bool addMessage(const T& msg)
    {
        bool ret = false;
        std::lock_guard<std::mutex> lock(sfMutex.mutex);
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
        std::lock_guard<std::mutex> lock(sfMutex.mutex);
        if (!messages.empty())
        {
            if (messages.front() == nullptr)
            {
                messages.pop_front();
            }
            else
            {
                T* casted = dynamic_cast<T*>(messages.front().get());
                if (casted)
                {
                    msg = *casted;
                    messages.pop_front();
                    ret = true;
                }
                else
                {
                    // Type mismatch - remove the invalid message
                    messages.pop_front();
                }
            }
        }

        return ret;
    }

    int32_t getNumMessages()
    {
        int32_t ret = 0;
        std::lock_guard<std::mutex> lock(sfMutex.mutex);
        ret = (int32_t)messages.size();

        return ret;
    }

    template <typename T>
    bool peekMessage(T& msg)
    {
        bool ret = false;
        std::lock_guard<std::mutex> lock(sfMutex.mutex);
        if (!messages.empty())
        {
            if (messages.front() == nullptr)
            {
                messages.pop_front();
            }
            else
            {
                T* casted = dynamic_cast<T*>(messages.front().get());
                if (casted)
                {
                    msg = *casted;
                    ret = true;
                }
            }
        }
        return ret;
    }

    bool getMessageType(SFType& type)
    {
        bool ret = false;
        std::lock_guard<std::mutex> lock(sfMutex.mutex);
        if (!messages.empty())
        {
            if (messages.front() == nullptr)
            {
                messages.pop_front();
            }
            else
            {
                type = messages.front()->getType();
                ret = true;
            }
        }
        return ret;
    }

    bool create_sock()
    {
        #if defined(_WIN32)
            if (!WinSockPair::get_sockPair(socks))
            {
                SFDebug::SF_print(std::string("SocketPair Creation failed"));
                return false;
            }
        #else
            if ((socketpair(AF_UNIX, SOCK_STREAM, 0, socks)) < 0)
            {
                SFDebug::SF_print(std::string("Socket Creation failed, Reason: ") + strerror(errno));
                return false;
            }
        #endif
        return true;
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

