#
# 	# proctest  project
#

cmake_minimum_required(VERSION 3.21)
include("${CMAKE_CURRENT_SOURCE_DIR}/../../buildsys/ccmake.cmake")

set (NO_VERSION_INFO 1)
set(PROJECT_NAME qwproctest)
set (NEED_BOOST  thread system filesystem date_time chrono)

commonStart()
set(INC_DIR_LIST "../" "../c_lib"  "../../../common" "../../../xmatrix2" )
	
addSourceFiles("source" "." qwproctest )	
addSourceFiles("source" "../proc/" qwproc.h)	
add_definitions(-DBOOST_DATE_TIME_NO_LIB)

if(UNIX)
	#addOtherLib("dl")
	if(UNIX AND NOT APPLE)
		addOtherLib("rt")
	endif()
endif()

programEnd()
if (VCPKG_EXISTS)
    install(TARGETS ${PROJECT_NAME}  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/tools/qwtw)
else()
    install(TARGETS ${PROJECT_NAME}  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
endif()

# ========================================================


