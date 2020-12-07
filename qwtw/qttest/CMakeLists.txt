#
# 	# qwproc  project
#

cmake_minimum_required (VERSION 3.1)
include("${CMAKE_CURRENT_SOURCE_DIR}/../../buildsys/ccmake.cmake")

#set (NO_VERSION_INFO 1)
set(PROJECT_NAME qttest)
#set (NEED_BOOST  thread system filesystem date_time chrono)

commonStart()
addQT(USE_CMD)
set(INC_DIR_LIST   "../../../common" )
	
addSourceFiles("source" "." qttest)	
addSourceFiles("qwtw" "../" plib1.qrc)
add_definitions(-DQT_DLL)

if(UNIX)
	add_definitions(-DLIN_UX)
    addOtherLib("dl")
    addOtherLib("rt")
endif()
if(WIN32)
	addOtherLib("ws2_32")
endif()

programEnd()

install(TARGETS ${PROJECT_NAME}  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)


# ========================================================


