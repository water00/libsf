#if defined(_WIN32)
   //define something for Windows (32-bit and 64-bit, this part is common)
   #include <WinSock2.h>
   #include <afunix.h>
   #define sleep(x) Sleep(x * 1000)
   #define strerror(x) std::to_string(WSAGetLastError())
   #define USE_SELECT
   #define sock_size int64_t
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
    #define sock_size int32_t
#elif __linux__
    // linux
    #include <errno.h>
    #include <sys/types.h>
    #include <pthread.h>
    #include <sys/socket.h>
    #include <sys/time.h>
    #include <unistd.h>
    #include <sys/epoll.h>
    #include <unistd.h>
    #define USE_EPOLL
    #define sock_size int32_t
#endif

