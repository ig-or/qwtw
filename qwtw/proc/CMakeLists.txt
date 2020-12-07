#
# 	# qwproc  project
#

cmake_minimum_required (VERSION 3.1)
include("${CMAKE_CURRENT_SOURCE_DIR}/../../buildsys/ccmake.cmake")

#set (NO_VERSION_INFO 1)
set(PROJECT_NAME qwproc)
set (NEED_BOOST  thread system filesystem date_time chrono)

commonStart()
addQT()
set(INC_DIR_LIST "../" "../c_lib"  "../../../common" "../../../xmatrix2" )
	
addSourceFiles("source" "." qwproc qwproto main)	
addSourceFiles("qwtw" "../" qworker  sfigure figure2 justaplot plib1.qrc)
addSourceFiles("common" "../../common"
	xstdef xqbytebuffer xmutils queue_block  xmroundbuf
	sqwlinetree coordtypes
)
addSourceFiles("klifft" "../../common/klifft"	complex fft psd )
addSourceFiles("matrix" "../../xmatrix2" xmatrix2 	xmatrixplatform xmatrix_imp xmcoords )
add_definitions(-DQT_DLL -DQWT_DLL)

if (WITH_UDP_SYNC)
	add_definitions(-DENABLE_UDP_SYNC)
endif()

add_definitions(-DXMPRINTF_TO_FILE)

if (USE_MARBLE)	
	add_definitions(-DUSEMARBLE)
	FIND_PACKAGE (marble REQUIRED)	
	list(APPEND INC_DIR_LIST ${MARBLE_INCLUDE_DIR}) 	
	FIND_LIBRARY(MARBLE_LIBRARY_Release NAMES marblewidget-qt5  )
	FIND_LIBRARY(MARBLE_LIBRARY_Debug NAMES marblewidget-qt5d  )

	#
	#message(STATUS "marble libraries: ${MARBLE_LIBRARIES}")

	addSourceFiles("marble" "../wmarble" marbleview)
	if(WIN32)
		#list(APPEND ${LR_LIST} optimized  "${vcpkg_root_folder}/installed/x64-windows/lib/marblewidget-qt5.lib")
		#list(APPEND ${LD_LIST} debug  "${vcpkg_root_folder}/installed/x64-windows/debug/lib/marblewidget-qt5d.lib")
		if (MARBLE_LIBRARY_Release)
			if (MARBLE_LIBRARY_Debug)
				list(APPEND ${LR_LIST} optimized  ${MARBLE_LIBRARY_Release})
				list(APPEND ${LD_LIST} debug   ${MARBLE_LIBRARY_Debug})
			else()
				list(APPEND ${L_LIST}   ${MARBLE_LIBRARY_Release})
				message(STATUS "can not find debug MARBLE library")

			endif()
		else()
			message(FATAL_ERROR "cannot licate marble library")
		endif()
	else()
		addOtherLib(${MARBLE_LIBRARIES})
	endif()
	
	add_definitions(-DQT_USE_QSTRINGBUILDER -DQT_NO_URL_CAST_FROM_STRING -DQT_NO_CAST_TO_ASCII -DQT_NO_CAST_FROM_BYTEARRAY -DQT_STRICT_ITERATORS)
	
endif()


############ QWT ###################
if(WIN32)
	FIND_PATH(QWT_INCLUDE_DIR NAMES qwt/qwt.h qwt.h)
	FIND_LIBRARY(QWT_LIBRARY_Release NAMES qwt  )
	FIND_LIBRARY(QWT_LIBRARY_Debug NAMES qwtd  )
	IF (QWT_INCLUDE_DIR AND QWT_LIBRARY_Release)
		SET(QWT_FOUND TRUE)
		if (QWT_LIBRARY_Debug)
			list(APPEND ${LR_LIST} optimized  ${QWT_LIBRARY_Release})
			list(APPEND ${LD_LIST} debug   ${QWT_LIBRARY_Debug})
		else()
			list(APPEND ${L_LIST}   ${QWT_LIBRARY_Release})
			message(STATUS "can not find debug QWT library")
		endif()
		list(APPEND INC_DIR_LIST ${QWT_INCLUDE_DIR} ${QWT_INCLUDE_DIR}/qwt)
		message(STATUS "qwt library added; QWT_INCLUDE_DIR = ${QWT_INCLUDE_DIR}; QWT_LIBRARY_Release = ${QWT_LIBRARY_Release}")
	else()
		message(FATAL_ERROR "cannot find QWT library! QWT_INCLUDE_DIR = ${QWT_INCLUDE_DIR}; QWT_LIBRARY_Release = ${QWT_LIBRARY_Release}")
	ENDIF()
	
else()
	add_definitions(-DLIN_UX)
	addQWTLinux()
	addOtherLib("rt")
	addOtherLib("dl")
	message("LIB_LIST: "  ${LIB_LIST})
endif()


if(UNIX)
    #addOtherLib("dl")
    #addOtherLib("rt")
endif()
if(WIN32)
	addOtherLib("ws2_32")
endif()

programEnd()
if(WIN32 AND MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS " /ENTRY:\"mainCRTStartup\" ")
endif()

if (VCPKG_EXISTS)
    install(TARGETS ${PROJECT_NAME}  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/tools/qwtw)
else()
    install(TARGETS ${PROJECT_NAME}  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
endif()

# ========================================================


