cmake_minimum_required(VERSION 3.21)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../buildsys/")
include(ccmake)

option(WITH_UDP_SYNC "UDP marker position broadcast"   OFF)
option(USE_MARBLE    "Marble-qt top-view support"      OFF)
option(USE_MGL       "MathGL2 3-D plotting"            OFF)

add_subdirectory(proc)
add_subdirectory(proclib)

if(USE_MGL)
    add_subdirectory(mathgl)
endif()

add_subdirectory(qttest)
add_subdirectory(qwtest2)
