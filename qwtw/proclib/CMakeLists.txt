cmake_minimum_required(VERSION 3.21)
include("${CMAKE_CURRENT_SOURCE_DIR}/../../buildsys/ccmake.cmake")

set(PROJECT_NAME qwtw)
commonStart()

addSourceFiles("source" "." qwtw)
addSourceFiles("lib" "../proc" qwproc pclient)
addSourceFiles("qrc" "../" qwtypes)
addSourceFiles("common" "../../common"
    xstdef xmutils
)

add_definitions(-DqqwtwEXPORTS -DqwtwcEXPORTS)

if(WITH_UDP_SYNC)
    add_definitions(-DENABLE_UDP_SYNC)
endif()
if(USE_MGL)
    add_definitions(-DUSEMATHGL)
endif()

if(UNIX AND NOT APPLE)
    add_definitions(-DLIN_UX)
    find_package(Threads REQUIRED)
    addOtherLib(${CMAKE_DL_LIBS} Threads::Threads)
elseif(APPLE)
    addOtherLib(${CMAKE_DL_LIBS})
elseif(WIN32)
    addOtherLib("advapi32" "shell32")
endif()

libraryEnd(SHARED)

if(MINGW)
    # Statically link libstdc++, libgcc, and winpthreads into libqwtw.dll (`-static`)
    # so loading it inside host runtimes that ship their own MinGW runtime DLLs
    # (e.g. Julia on Windows) never clashes on libstdc++-6.dll or libwinpthread-1.dll.
    target_link_options(${PROJECT_NAME} PRIVATE -static)
endif()

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
