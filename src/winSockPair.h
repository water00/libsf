
#pragma once
#include <iostream>
#include <stdint.h>
#include <WinSock2.h>
#include <afunix.h>
#include <cstring>
#include <future>
#include <chrono>


class WinSockPair
{
private:
    inline static bool initDone {false};
    inline static int64_t srvSock {-1};
    inline static const int32_t winBacklog {8};
    inline static const char* winSrvPath  {"/temp/socket.sock"};

    WinSockPair(const WinSockPair&) = delete;
    WinSockPair& operator=(const WinSockPair&) = delete;
    WinSockPair() = delete;

    static void init_sockPair()
    {
        WSADATA wsaData;
        int wsaRes;
        struct sockaddr_un servaddr;
  
        _unlink(winSrvPath);
        memset((char*)&servaddr, 0, sizeof(servaddr));
        servaddr.sun_family = AF_UNIX;
        strcpy_s(servaddr.sun_path, sizeof(servaddr.sun_path), winSrvPath);


        if ((wsaRes = WSAStartup(MAKEWORD(2,2), &wsaData)) != 0) 
        {
            std::cout << "WSAStartup failed with error: " <<  wsaRes << std::endl;
        }
        else if ((srvSock = socket(AF_UNIX, SOCK_STREAM, 0)) == INVALID_SOCKET)
        {
            std::cout << "Socket creation failed." << std::endl;
        }
        else if ((bind(srvSock, (sockaddr*)&servaddr, sizeof(servaddr))) == SOCKET_ERROR)
        {
            std::cout << "Bind to socket failed" << std::endl;
        }
        else if ((listen(srvSock, winBacklog)) != 0)
        {
            std::cout << "Listen failed" << std::endl;
        }
        else
        {
            initDone = true;
            std::cout << "Init Done" << std::endl;
        }
    }

    static int64_t get_acceptSock()
    {
        int64_t acceptSock;

        if ((acceptSock = accept(srvSock, NULL, NULL)) < 0)
        {
            std::cout << "acceptSock failed" << std::endl;
            return -1;
        }
        return acceptSock;
    }

    static int64_t get_clientSock()
    {
        int64_t cliSock;
        struct sockaddr_un cliaddr;

        if ((cliSock = socket(AF_UNIX, SOCK_STREAM, 0)) == INVALID_SOCKET)
        {
            std::cout << "Client sock failed" << std::endl;
            return -1;
        }
        memset(&cliaddr, 0, sizeof(cliaddr));
        cliaddr.sun_family = AF_UNIX;
        strcpy_s(cliaddr.sun_path, sizeof(cliaddr.sun_path), winSrvPath);

        if (connect(cliSock, (sockaddr*)&cliaddr, sizeof(cliaddr)) == SOCKET_ERROR)
        {
            std::cout << "Client connect failed" << std::endl;
        }
        return cliSock;
    }

public:
    static bool get_sockPair(int32_t socks[2])
    {
        bool ret = true;
        if (!initDone)
        {
            init_sockPair();
            if (!initDone)
            {
                std::cout << "Init Not done" << std::endl;
                return false;
            }
        }

        std::future<int64_t> srvFut = std::async(&WinSockPair::get_acceptSock);
        std::future<int64_t> cliFut = std::async(&WinSockPair::get_clientSock);

        // Casting 64 bit to 32 bit; Normally shouldn't be a problem
        // Caution warrented.
        if ((socks[0] = (int32_t)srvFut.get()) == INVALID_SOCKET)
        {
            ret = false;
        }
        if ((socks[1] = (int32_t)cliFut.get()) == INVALID_SOCKET)
        {
            ret = false;
        }
        return ret;
    }
};