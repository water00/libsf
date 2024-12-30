if (WIN32)
    find_library (
        VLD_LIB
        PATHS "C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
        NAMES vld.lib
    )
    message(STATUS "VLD_LIB: [${VLD_LIB}]")
    if (NOT VLD_LIB)
        message("Did not find vld.lib")
    endif()
endif()


add_subdirectory(debugCheck)
add_subdirectory(multiTask)
add_subdirectory(messagePong)
add_subdirectory(stateCheck)
add_subdirectory(timerCheck)
