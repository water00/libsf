

#cmake_minimum_required(VERSION 3.10)

#project(multiTask VERSION 1.0)

# Need C++ 11
#set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_STANDARD_REQUIRED True)

#set (LIBSF_DIR "c:/program files \(x86\)/libsf")

set (TARGET_NAME multiTask)

add_executable(${TARGET_NAME} 
    taskChkMain.cpp
    task.cpp
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

target_link_libraries(${TARGET_NAME} libsf)

#target_include_directories(${TARGET_NAME} 
#    PUBLIC ${LIBSF_DIR}/include/
#)