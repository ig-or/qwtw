cmake_minimum_required(VERSION 3.21)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
project(qwtw LANGUAGES C CXX)

add_subdirectory(extern)
add_subdirectory(qwtw)
