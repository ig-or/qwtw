cmake_minimum_required(VERSION 3.21)
include("${CMAKE_CURRENT_SOURCE_DIR}/../../buildsys/ccmake.cmake")

set(PROJECT_NAME qwproc)
set(NEED_BOOST thread filesystem date_time chrono program_options)

commonStart()
addQT()

set(INC_DIR_LIST "../" "../c_lib" "../../../common" "../../../xmatrix2")

addSourceFiles("source" "." qwproc qwproto main)
addSourceFiles("qwtw" "../" qworker sfigure figure2 justaplot qwtypes line settings spectrogram qmarkers)
addSourceFiles("qwtw" "../nanoflann" nanoflann)
addSourceFiles("qrc" "../../img" plib1.qrc)
addSourceFiles("common" "../../common"
    xstdef xqbytebuffer xmutils queue_block xmroundbuf
    sqwlinetree coordtypes
)
addSourceFiles("klifft" "../../common/klifft" complex fft psd)
addSourceFiles("matrix" "../../xmatrix2" xmatrix2 xmatrixplatform xmatrix_imp xmcoords)

add_definitions(-DQT_DLL -DQWT_DLL)

if(WITH_UDP_SYNC)
    add_definitions(-DENABLE_UDP_SYNC)
endif()

add_definitions(-DXMPRINTF_TO_FILE)

if(USE_MGL)
    add_definitions(-DUSEMATHGL)
    addSourceFiles("mgl" "../mathgl" qmgl qmglplot)
    find_package(MathGL2 REQUIRED Qt)
    list(APPEND INC_DIR_LIST ${MATHGL2_INCLUDE_DIRS} ../../)
    addOtherLib(${MATHGL2_LIBRARIES})
    if(UNIX AND NOT APPLE)
        addOtherLib("GL")
    elseif(APPLE)
        find_library(OPENGL_LIB OpenGL)
        if(OPENGL_LIB)
            addOtherLib(${OPENGL_LIB})
        endif()
    endif()
endif()

if(USE_MARBLE)
    add_definitions(-DUSEMARBLE)
    addSourceFiles("marble" "../wmarble" marbleview)
    find_package(Marble REQUIRED)
    find_library(MARBLE_LIBRARY_Release NAMES marblewidget-qt5)
    addOtherLib(${MARBLE_LIBRARIES})
    list(APPEND INC_DIR_LIST ${MARBLE_INCLUDE_DIR})
    add_definitions(-DQT_USE_QSTRINGBUILDER -DQT_NO_URL_CAST_FROM_STRING
        -DQT_NO_CAST_TO_ASCII -DQT_NO_CAST_FROM_BYTEARRAY -DQT_STRICT_ITERATORS)
endif()

# --- Qwt ---
addQWT()

# --- platform libs ---
if(UNIX AND NOT APPLE)
    addOtherLib("rt" "dl")
    find_package(OpenSSL REQUIRED)
    addOtherLib(${OPENSSL_LIBRARIES})
    list(APPEND INC_DIR_LIST ${OPENSSL_INCLUDE_DIR})
elseif(APPLE)
    addOtherLib("dl")
    find_package(OpenSSL REQUIRED)
    addOtherLib(${OPENSSL_LIBRARIES})
    list(APPEND INC_DIR_LIST ${OPENSSL_INCLUDE_DIR})
elseif(WIN32)
    addOtherLib("ws2_32")
endif()

programEnd()

if(WIN32 AND MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS " /ENTRY:\"mainCRTStartup\" ")
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
