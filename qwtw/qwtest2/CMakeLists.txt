#
# 	# qwtwc library test2 project
#

cmake_minimum_required (VERSION 2.8.11)
include("${CMAKE_CURRENT_SOURCE_DIR}/../../buildsys/ccmake.cmake")
set(USE_QT YES)

set(PROJECT_NAME qwtest2)
set (NEED_BOOST  thread system filesystem date_time chrono)
commonStart()
set(NO_VERSION_INFO 1) 
#add_definitions(-DUSEMARBLE)

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


