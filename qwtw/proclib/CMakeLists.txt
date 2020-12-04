#
# 	# qwtw proclib library project
#

cmake_minimum_required (VERSION 2.8.11)
include("${CMAKE_CURRENT_SOURCE_DIR}/../../buildsys/ccmake.cmake")

set(PROJECT_NAME qwtw)
commonStart()
#set (NO_VERSION_INFO 1)

#if (WITH_QT3D)
#	set(USE_QT3D YES) 
#endif()

set (NEED_BOOST  thread system filesystem date_time chrono regex)
	
addSourceFiles("source" "." qwtw )	
addSourceFiles("lib" "../proc" qwproc pclient )	
addSourceFiles("${PROJECT_NAME}" "../" plib1.qrc)	

addSourceFiles("common" "../../common"
	xstdef xmutils
)

#add_definitions(-DUNICODE -D_UNICODE)
add_definitions(-DqqwtwEXPORTS -DqwtwcEXPORTS)
#add_definitions(-DXMPRINTF_TO_FILE)
if (WITH_UDP_SYNC)
	add_definitions(-DENABLE_UDP_SYNC)
endif()

if (USE_MARBLE)	
	add_definitions(-DUSEMARBLE)
endif()

#if (WITH_QT3D)
#	add_definitions(-DUSE_QT3D)
#	addSourceFiles("qt-3d" "../qt-3d"   qt-3d)
#endif()

if(WIN32)

	
else()
	add_definitions(-DLIN_UX)
	addOtherLib("rt")
	addOtherLib("dl")
	#message("LIB_LIST: "  ${LIB_LIST})
endif()
libraryEnd(SHARED)

set(OUR_LIBNAME_REPORT ${OUR_LIBNAME_REPORT}  ${PROJECT_NAME} CACHE INTERNAL "OUR_LIBNAME_REPORT")

if(WIN32)
  INSTALL(TARGETS ${PROJECT_NAME}  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
elseif(APPLE)
  # not supported
else()
  install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION  ${CMAKE_INSTALL_PREFIX}/lib  #${CMAKE_INSTALL_LIBDIR}
  )
endif()

INSTALL(FILES
    qwtw.h    
	DESTINATION  ${CMAKE_INSTALL_PREFIX}/include  
	CONFIGURATIONS Release 
)

