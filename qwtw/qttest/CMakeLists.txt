cmake_minimum_required(VERSION 3.21)
include("${CMAKE_CURRENT_SOURCE_DIR}/../../buildsys/ccmake.cmake")

set(PROJECT_NAME qttest)
set(NEED_BOOST filesystem date_time program_options thread)

commonStart()
addQT(USE_CMD YES)

set(INC_DIR_LIST "../../../common")

addSourceFiles("source" "." qttest)
addSourceFiles("qrc" "../../img" plib1.qrc)
addSourceFiles("common" "../../common"
    xstdef xmutils
)

add_definitions(-DQT_DLL)

if(USE_MARBLE)
    add_definitions(-DUSEMARBLE)
    find_package(Marble REQUIRED)
    find_library(MARBLE_LIBRARY_Release NAMES marblewidget-qt5)
    addOtherLib(${MARBLE_LIBRARIES})
    list(APPEND INC_DIR_LIST ${MARBLE_INCLUDE_DIR})
    add_definitions(-DQT_USE_QSTRINGBUILDER -DQT_NO_URL_CAST_FROM_STRING
        -DQT_NO_CAST_TO_ASCII -DQT_NO_CAST_FROM_BYTEARRAY -DQT_STRICT_ITERATORS)
endif()

# --- Qwt + platform libs ---
if(UNIX)
    addQWT()
    if(NOT APPLE)
        add_definitions(-DLIN_UX)
        addOtherLib("rt" "dl")
    else()
        addOtherLib("dl")
    endif()
    find_package(OpenSSL REQUIRED)
    addOtherLib(${OPENSSL_LIBRARIES})
    list(APPEND INC_DIR_LIST ${OPENSSL_INCLUDE_DIR})
elseif(WIN32)
    addOtherLib("ws2_32")
endif()

programEnd()

if(WIN32 AND MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS " /SUBSYSTEM:CONSOLE /ENTRY:\"mainCRTStartup\" ")
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
