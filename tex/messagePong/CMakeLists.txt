
set (TARGET_NAME messagePong)

add_executable(${TARGET_NAME} 
    taskMain.cpp
    task1.cpp
    task2.cpp
)

if(WIN32)
    target_compile_features(${TARGET_NAME} PRIVATE cxx_std_17)
    target_link_libraries(${TARGET_NAME} ws2_32.lib Mswsock.lib AdvApi32.lib)
else()
    target_link_libraries(${TARGET_NAME} lpthread)
endif()

target_link_libraries(${TARGET_NAME} libsf)

