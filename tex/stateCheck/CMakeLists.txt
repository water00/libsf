set (TARGET_NAME stateCheck)

add_executable(${TARGET_NAME} 
    stateCheckMain.cpp
    stateTask.cpp
)

if(WIN32)
    target_compile_features(${TARGET_NAME} PRIVATE cxx_std_17)
    target_link_libraries(${TARGET_NAME} ws2_32.lib Mswsock.lib AdvApi32.lib)
else()
    target_link_libraries(${TARGET_NAME} lpthread)
endif()

target_link_libraries(${TARGET_NAME} libsf)


