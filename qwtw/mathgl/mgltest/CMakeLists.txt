#
# 	# MGL_QT test   project
#

cmake_minimum_required (VERSION 3.1)
include("${CMAKE_CURRENT_SOURCE_DIR}/../../../buildsys/ccmake.cmake")
set (NO_VERSION_INFO 1)

#set (NO_VERSION_INFO 1)
set(PROJECT_NAME mgltest)
#set (NEED_BOOST  thread system filesystem date_time chrono)
set (NEED_BOOST system filesystem date_time  program_options)

commonStart()
addQT(USE_CMD YES)
set(INC_DIR_LIST   "../../../common" )
	
addSourceFiles("source" "." mgltest)	
addSourceFiles("mgl" "../" qmgl)
addSourceFiles("base" "../../" qwtypes line)
addSourceFiles("qrc" "../../../img" plib1.qrc)	
addSourceFiles("common" "../../../common"
	xstdef xmutils sqwlinetree
)

addSourceFiles("matrix" "../../../xmatrix2" xmatrix_imp xmatrix2)


add_definitions(-DQT_DLL)

find_package(MathGL2 REQUIRED Qt)
list(APPEND INC_DIR_LIST ${MATHGL2_INCLUDE_DIRS})
list(APPEND INC_DIR_LIST "${CMAKE_CURRENT_SOURCE_DIR}/../../../") 
addOtherLib(${MATHGL2_LIBRARIES})
message(STATUS "got MATHGL2_VERSION_STRING ${MATHGL2_VERSION_STRING}")

if(UNIX)
	add_definitions(-DLIN_UX)
	addQWTLinux() # do not need this actually here....
	addOtherLib("rt")
	addOtherLib("dl")
	addOtherLib("GL")

	message("STATUS LIB_LIST: "  ${${L_LIST}})
endif()
if(WIN32)
	addOtherLib("ws2_32")
	addOtherLib("opengl32")
endif()

programEnd()
if(WIN32 AND MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS " /SUBSYSTEM:CONSOLE  /ENTRY:\"mainCRTStartup\" ")
endif()

install(TARGETS ${PROJECT_NAME}  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)


# ========================================================


