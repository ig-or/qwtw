#
# 	# qwtwc library test2 project
#

cmake_minimum_required (VERSION 3.16.0)
include("${CMAKE_CURRENT_SOURCE_DIR}/../../buildsys/ccmake.cmake")
#set(USE_QT YES)

set(PROJECT_NAME qwtest2)
set (NEED_BOOST  thread system filesystem date_time chrono program_options)
commonStart()
set(NO_VERSION_INFO 1) 

if (USE_MARBLE)	
	add_definitions(-DUSEMARBLE)
endif()

if (USE_MGL)
	add_definitions(-DUSEMATHGL)
endif()

set(INC_DIR_LIST "../" "../c_lib"  "../../../common" "../../../xmatrix2" )
	
addSourceFiles("source" "../" qwtest2)	
if(UNIX)
    addOtherLib("dl")
    addOtherLib("rt")
endif()
programEnd()

if (VCPKG_EXISTS)
    install(TARGETS ${PROJECT_NAME}  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/tools/qwtw)
else()
    install(TARGETS ${PROJECT_NAME}  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
endif()

# ========================================================


