#pragma once

#include <string>

#if defined(_WIN32)
    //define something for Windows (32-bit and 64-bit, this part is common)
    // Windows restricts the size to 64. Increase it.
    #ifndef FD_SETSIZE
    #define FD_SETSIZE 8192
    #endif
    #define sleep(x) Sleep(x * 1000)
    // Note: WSAGetLastError() returns the last Winsock error;
    // the parameter is accepted but unused for API compatibility.
    #define strerror(x) std::to_string(WSAGetLastError())
    #define USE_SELECT
    using sock_size = int64_t;
    #include <WinSock2.h>
    #include <afunix.h>
    #include "winSockPair.h"
    #ifdef VLD
        #include <vld.h>
    #endif
    #ifdef _WIN64
        //define something for Windows (64-bit only)
    #else
        //define something for Windows (32-bit only)
    #endif
#elif __APPLE__
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR
         // iOS Simulator
    #elif TARGET_OS_IPHONE
        // iOS device
    #elif TARGET_OS_MAC
        // Other kinds of Mac OS
        #include <errno.h>
        #include <sys/types.h>
        #include <pthread.h>
        #include <sys/socket.h>
        #include <sys/time.h>
        #include <unistd.h>
    #else
    #   error "Unknown Apple platform"
    #endif
    #define USE_SELECT
    using sock_size = int32_t;
#elif __linux__
    // linux
    #include <errno.h>
    #include <sys/types.h>
    #include <pthread.h>
    #include <sys/socket.h>
    #include <sys/time.h>
    #include <unistd.h>
    #include <sys/epoll.h>
    #define USE_EPOLL
    using sock_size = int32_t;
#endif

