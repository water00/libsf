add_library(libsf INTERFACE)

target_include_directories(libsf INTERFACE ...)
target_sources(libsf INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/osRelated.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sfDebug.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sfMessages.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sfMutex.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sfState.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sfTask.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sfThreadBase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sfThread_epoll.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sfThread_select.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sfThread.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sfTimer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/winSockPair.h
)

install(FILES 
    osRelated.h
    sfDebug.h
    sfMessages.h
    sfMutex.h
    sfState.h
    sfTask.h
    sfThreadBase.h
    sfThread_epoll.h
    sfThread_select.h
    sfThread.h
    sfTimer.h
    winSockPair.h
    DESTINATION include)
