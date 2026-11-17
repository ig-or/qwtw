cmake_minimum_required(VERSION 3.21)
include("${CMAKE_CURRENT_SOURCE_DIR}/../../buildsys/ccmake.cmake")

set(PROJECT_NAME qttest)

commonStart()
addQT(USE_CMD YES)

set(INC_DIR_LIST "../../../common")

addSourceFiles("source" "." qttest)
addSourceFiles("qrc" "../../img" plib1.qrc)
addSourceFiles("common" "../../common"
    xstdef xmutils
)

add_definitions(-DQT_DLL)

# --- Qwt + platform libs ---
if(UNIX)
    addQWT()
    if(NOT APPLE)
        add_definitions(-DLIN_UX)
        find_package(Threads REQUIRED)
        addOtherLib(${CMAKE_DL_LIBS} Threads::Threads)
    else()
        addOtherLib(${CMAKE_DL_LIBS})
    endif()
elseif(WIN32)
    addOtherLib("ws2_32")
endif()

programEnd()

if(WIN32 AND MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS " /SUBSYSTEM:CONSOLE /ENTRY:\"mainCRTStartup\" ")
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
