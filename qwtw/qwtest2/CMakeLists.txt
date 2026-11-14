cmake_minimum_required(VERSION 3.21)
include("${CMAKE_CURRENT_SOURCE_DIR}/../../buildsys/ccmake.cmake")

set(PROJECT_NAME qwtest2)
commonStart()
set(NO_VERSION_INFO 1)

if(USE_MGL)
    add_definitions(-DUSEMATHGL)
endif()

set(INC_DIR_LIST "../" "../proclib" "../../../common" "../../../xmatrix2")

addSourceFiles("source" "../" qwtest2)

if(UNIX AND NOT APPLE)
    find_package(Threads REQUIRED)
    addOtherLib("dl" Threads::Threads)
elseif(APPLE)
    addOtherLib("dl")
endif()

programEnd()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
