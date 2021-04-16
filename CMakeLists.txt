cmake_minimum_required(VERSION 3.10)

project(libsf VERSION 1.0)

# Configure file
configure_file(libsfConfig.h.in libsfConfig.h)

# Need C++ 11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(src)
add_subdirectory(tex)

