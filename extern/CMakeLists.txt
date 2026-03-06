# CMake wrapper to build vendored Qwt 6.3 from source.
# Qwt upstream uses qmake; this file replaces it with a pure-CMake build.

cmake_minimum_required(VERSION 3.21)

set(QWT_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/qwt/src")

file(GLOB QWT_SOURCES "${QWT_SRC_DIR}/*.cpp")
file(GLOB QWT_HEADERS "${QWT_SRC_DIR}/*.h")

# qwt_plot_glcanvas uses QGLWidget (<qgl.h>), removed in Qt6.
list(FILTER QWT_SOURCES EXCLUDE REGEX "qwt_plot_glcanvas\\.cpp$")
list(FILTER QWT_HEADERS EXCLUDE REGEX "qwt_plot_glcanvas\\.h$")

# SVG support requires Qt6::Svg; skip it to reduce dependencies.
list(FILTER QWT_SOURCES EXCLUDE REGEX "qwt_plot_svgitem\\.cpp$")
list(FILTER QWT_HEADERS EXCLUDE REGEX "qwt_plot_svgitem\\.h$")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets Concurrent PrintSupport)

# Apply Qt6.10 compatibility patches to the vendored Qwt source.
find_package(Python3 REQUIRED COMPONENTS Interpreter)
execute_process(
    COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/qt6_patch.py"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    RESULT_VARIABLE _patch_result
)
if(NOT _patch_result EQUAL 0)
    message(WARNING "qt6_patch.py returned ${_patch_result}")
endif()

add_library(qwt STATIC ${QWT_SOURCES} ${QWT_HEADERS})

set_target_properties(qwt PROPERTIES
    AUTOMOC ON
    POSITION_INDEPENDENT_CODE ON
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(qwt
    PUBLIC  "${QWT_SRC_DIR}"
)

# QWT_MOC_INCLUDE=1 tells Qwt .cpp files to #include their own moc output,
# which ensures MOC-generated code sees all necessary includes.
target_compile_definitions(qwt
    PUBLIC  QWT_NO_SVG
    PRIVATE QWT_NO_OPENGL=$<NOT:$<BOOL:${QT_OPENGL_FOUND}>>
            QWT_MOC_INCLUDE=1
)

target_link_libraries(qwt
    PUBLIC
        Qt6::Core Qt6::Gui Qt6::Widgets
        Qt6::OpenGL Qt6::OpenGLWidgets
        Qt6::Concurrent Qt6::PrintSupport
)
