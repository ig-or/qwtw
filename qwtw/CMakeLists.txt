#
# 	# qwtw library project
#

cmake_minimum_required (VERSION 2.8.11)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../buildsys/")
include(ccmake)

option(WITH_UDP_SYNC "use UDP port for marker position broadcast"  ON)
option(USE_MARBLE "use marble-qt for 'top-view' data" OFF)

add_subdirectory(proc)
add_subdirectory(c_lib)
add_subdirectory(qwtest2)
add_subdirectory(c_lib_test)

