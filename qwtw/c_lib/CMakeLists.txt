#
# 	# qwtwc library project
#

cmake_minimum_required (VERSION 2.8.11)
include("${CMAKE_CURRENT_SOURCE_DIR}/../../buildsys/ccmake.cmake")

option(WITH_UDP_SYNC "use UDP port for marker position broadcast"  ON)
option(USE_MARBLE "use marble-qt for 'top-view' data" ON)
#option(WITH_QT3D "3D plotting support from QT" OFF)

set(PROJECT_NAME qwtw)
commonStart()
addQT()
set (NO_VERSION_INFO 1)

#if (WITH_QT3D)
#	set(USE_QT3D YES) 
#endif()

set (NEED_BOOST  thread system filesystem date_time chrono regex)
	
addSourceFiles("source" "." 	qtint qwtw )	
addSourceFiles("${PROJECT_NAME}" "../" sfigure figure2 justaplot plib1.qrc)	
addSourceFiles("matrix" "../../xmatrix2" xmatrix2 	xmatrixplatform xmatrix_imp xmcoords )

addSourceFiles("common" "../../common"
	xstdef xqbytebuffer xmutils queue_block  xmroundbuf
	sqwlinetree coordtypes
)

addSourceFiles("klifft" "../../common/klifft"	complex fft psd )

add_definitions(-DUNICODE -D_UNICODE)
add_definitions(-DQT_DLL -DQWT_DLL -DqqwtwEXPORTS -DqwtwcEXPORTS)
if (WITH_UDP_SYNC)
	add_definitions(-DENABLE_UDP_SYNC)
endif()


if (USE_MARBLE)	
	add_definitions(-DUSEMARBLE)
	if (VCPKG_EXISTS)
		message(STATUS "VCPKG_EXISTS active")
		if(DEFINED ENV{VCPKG_ROOT})
			set( vcpkg_root_folder $ENV{VCPKG_ROOT})
			message(STATUS "got vcpkg_root_folder from ENV:   ${vcpkg_root_folder}")
		else()
			if (CMAKE_TRY_COMPILE_PLATFORM_VARIABLES)
				list(GET CMAKE_TRY_COMPILE_PLATFORM_VARIABLES -1 vcpkg_root_folder)
				set(vcpkg_root_folder ${${vcpkg_root_folder}})
				message(STATUS "got vcpkg_root_folder from CMAKE_TRY_COMPILE_PLATFORM_VARIABLES:   ${vcpkg_root_folder}")
			else()
				if (_VCPKG_ROOT_DIR)
					set( vcpkg_root_folder ${_VCPKG_ROOT_DIR})
					message(STATUS "got vcpkg_root_folder from _VCPKG_ROOT_DIR:     ${vcpkg_root_folder}")
				else()
					# TODO: try something else here, like searching for .vcpkg-root file
					message(FATAL_ERROR "cannot fine vcpkg root folder")
				endif()
			endif()
		endif()

		if (vcpkg_root_folder)
			message(STATUS "vcpkg_root_folder:   ${vcpkg_root_folder} ")
			if(WIN32)
				file(TO_CMAKE_PATH "${vcpkg_root_folder}/installed/x64-windows/tools/marble/data" marble_data_path_v1)
				message(STATUS  "marble data path: ${marble_data_path_v1}")
				add_definitions(-DVCPKG_MARBLE_DATA_PATH=${marble_data_path_v1})
			else()  #  LINUX?
				#message()
			endif()
		else()
			message(FATAL_ERROR "VCPKG_ROOT:  NOT DEFINED ")
		endif()
	else()
		message(STATUS "VCPKG_EXISTS FALSE; VCPKG_ROOT = $ENV{VCPKG_ROOT} ;   VCPKG_TOOLCHAIN = ${VCPKG_TOOLCHAIN}")
	endif()
	addSourceFiles("marble" "../wmarble" marbleview)
	
	add_definitions(-DQT_USE_QSTRINGBUILDER -DQT_NO_URL_CAST_FROM_STRING -DQT_NO_CAST_TO_ASCII -DQT_NO_CAST_FROM_BYTEARRAY -DQT_STRICT_ITERATORS)
	
	FIND_PACKAGE (marble REQUIRED)	
	list(APPEND INC_DIR_LIST ${MARBLE_INCLUDE_DIR}) 	
	addOtherLib(${MARBLE_LIBRARIES})
	message(STATUS "marble libraries: ${MARBLE_LIBRARIES}")
endif()


#if (WITH_QT3D)
#	add_definitions(-DUSE_QT3D)
#	addSourceFiles("qt-3d" "../qt-3d"   qt-3d)
#endif()

if(WIN32)
	FIND_PATH(QWT_INCLUDE_DIR NAMES qwt/qwt.h)
	FIND_LIBRARY(QWT_LIBRARY NAMES qwt qwt )
	IF (QWT_INCLUDE_DIR AND QWT_LIBRARY)
		SET(QWT_FOUND TRUE)
		list(APPEND ${L_LIST} ${QWT_LIBRARY})
		list(APPEND INC_DIR_LIST ${QWT_INCLUDE_DIR} ${QWT_INCLUDE_DIR}/qwt)
		message(STATUS "qwt library added; QWT_INCLUDE_DIR = ${QWT_INCLUDE_DIR}; QWT_LIBRARY = ${QWT_LIBRARY}")
	else()
		message(FATAL_ERROR "cannot find QWT library! QWT_INCLUDE_DIR = ${QWT_INCLUDE_DIR}; QWT_LIBRARY = ${QWT_LIBRARY}")
	ENDIF()
	
else()
	add_definitions(-DLIN_UX)
	addQWTLinux()
	message("LIB_LIST: "  ${LIB_LIST})
endif()
libraryEnd(SHARED)

set(OUR_LIBNAME_REPORT ${OUR_LIBNAME_REPORT}  ${PROJECT_NAME} CACHE INTERNAL "OUR_LIBNAME_REPORT")

if(WIN32)
	#copy_pb("C:/ProgramData/qwtw")
endif()


if(WIN32)
  INSTALL(TARGETS ${PROJECT_NAME}  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
elseif(APPLE)
  # not supported
else()
  install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )
endif()

INSTALL(FILES
    qwtw.h    
	DESTINATION  include  CONFIGURATIONS Release 
)

