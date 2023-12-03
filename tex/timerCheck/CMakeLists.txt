set (TARGET_NAME timerCheck)

add_executable(${TARGET_NAME} 
    timerCheckMain.cpp
    timerTask.cpp
)

if(WIN32)
    target_compile_features(${TARGET_NAME} PRIVATE cxx_std_17)
    target_link_libraries(${TARGET_NAME} ws2_32.lib Mswsock.lib AdvApi32.lib)
    if (VLD_LIB)
        target_link_libraries(${TARGET_NAME} ${VLD_LIB})
        target_compile_definitions(${TARGET_NAME} PUBLIC VLD)
    endif()
else()
    target_link_libraries(${TARGET_NAME} lpthread)
endif()

#target_link_libraries(${TARGET_NAME} libsf)

target_include_directories(${TARGET_NAME}
    PUBLIC ${LIBSF_DIR}
)

