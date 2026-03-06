cmake_minimum_required(VERSION 3.21)
include("${CMAKE_CURRENT_SOURCE_DIR}/../../buildsys/ccmake.cmake")

set(PROJECT_NAME qwtw)
commonStart()

set(NEED_BOOST thread filesystem date_time chrono regex)

addSourceFiles("source" "." qwtw)
addSourceFiles("lib" "../proc" qwproc pclient)
addSourceFiles("qrc" "../../img" plib1.qrc)
addSourceFiles("qrc" "../" qwtypes)
addSourceFiles("common" "../../common"
    xstdef xmutils
)

add_definitions(-DqqwtwEXPORTS -DqwtwcEXPORTS)

if(WITH_UDP_SYNC)
    add_definitions(-DENABLE_UDP_SYNC)
endif()
if(USE_MARBLE)
    add_definitions(-DUSEMARBLE)
endif()
if(USE_MGL)
    add_definitions(-DUSEMATHGL)
endif()

if(UNIX AND NOT APPLE)
    add_definitions(-DLIN_UX)
    addOtherLib("rt" "dl")
elseif(APPLE)
    addOtherLib("dl")
endif()

libraryEnd(SHARED)

set(OUR_LIBNAME_REPORT ${OUR_LIBNAME_REPORT} ${PROJECT_NAME} CACHE INTERNAL "OUR_LIBNAME_REPORT")

if(WIN32)
    install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
        ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
else()
    install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
endif()

install(FILES qwtw.h
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include
    CONFIGURATIONS Release)
