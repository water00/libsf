
add_executable(debugCheck 
    debugCheckMain.cpp
)

target_link_libraries(debugCheck
    libsf
)

target_include_directories(debugCheck
    PUBLIC ${CMAKE_SOURCE_DIR}/src
)
