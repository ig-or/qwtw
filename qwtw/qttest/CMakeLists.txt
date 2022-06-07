#
# 	# QT test   project
#

cmake_minimum_required (VERSION 3.1)
include("${CMAKE_CURRENT_SOURCE_DIR}/../../buildsys/ccmake.cmake")

#set (NO_VERSION_INFO 1)
set(PROJECT_NAME qttest)
#set (NEED_BOOST  thread system filesystem date_time chrono)
set (NEED_BOOST system filesystem date_time  program_options thread)

commonStart()
addQT(USE_CMD YES)
set(INC_DIR_LIST   "../../../common" )
	
addSourceFiles("source" "." qttest)	
addSourceFiles("qrc" "../../img" plib1.qrc)	
addSourceFiles("common" "../../common"
	xstdef xmutils
)

add_definitions(-DQT_DLL)

if (USE_MARBLE)	
	add_definitions(-DUSEMARBLE)
	#addSourceFiles("marble" "../wmarble" marbleview)

	FIND_PACKAGE(Marble REQUIRED)	
	#list(APPEND INC_DIR_LIST ${MARBLE_INCLUDE_DIR}) 	
	
	FIND_LIBRARY(MARBLE_LIBRARY_Release NAMES marblewidget-qt5  )
	FIND_LIBRARY(MARBLE_LIBRARY_Debug NAMES marblewidget-qt5d  )
	FIND_PATH(MARBLE_INCLUDE_DIR_1 NAMES marble/MarbleGlobal.h MarbleGlobal.h)

	message(STATUS "MARBLE_LIBRARY_Release = ${MARBLE_LIBRARY_Release}")
	message(STATUS "MARBLE_LIBRARY_Debug = ${MARBLE_LIBRARY_Debug}")
	message(STATUS "MARBLE_INCLUDE_DIR_1 = ${MARBLE_INCLUDE_DIR_1}")
	message(STATUS "MARBLE_INCLUDE_DIR = ${MARBLE_INCLUDE_DIR}")
	message(STATUS "MARBLE_LIBRARIES = ${MARBLE_LIBRARIES}")
	message(STATUS "Marble = ${Marble}")

	addOtherLib(${MARBLE_LIBRARIES})
	list(APPEND INC_DIR_LIST ${MARBLE_INCLUDE_DIR}) 	
	add_definitions(-DQT_USE_QSTRINGBUILDER -DQT_NO_URL_CAST_FROM_STRING -DQT_NO_CAST_TO_ASCII -DQT_NO_CAST_FROM_BYTEARRAY -DQT_STRICT_ITERATORS)
endif()


if(UNIX)
	add_definitions(-DLIN_UX)
	addQWTLinux()
	addOtherLib("rt")
	addOtherLib("dl")

	find_package(OpenSSL REQUIRED)
	message("STATUS OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
    message("STATUS OpenSSL libraries: ${OPENSSL_LIBRARIES}")
	addOtherLib(${OPENSSL_LIBRARIES})
	list(APPEND INC_DIR_LIST ${OPENSSL_INCLUDE_DIR}) 

	message("STATUS LIB_LIST: "  ${${L_LIST}})
endif()
if(WIN32)
	addOtherLib("ws2_32")
endif()

programEnd()
if(WIN32 AND MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS " /SUBSYSTEM:CONSOLE  /ENTRY:\"mainCRTStartup\" ")
endif()

install(TARGETS ${PROJECT_NAME}  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)


# ========================================================


