
# Build-system helper macros for qwtw
#    \file ccmake.cmake
#    \author   Igor Sandler
#
# Modernized for CMake 3.21+, macOS + Linux.
# Windows support preserved but not actively tested in this pass.
#
# Variables consumed by these macros:
#   NEED_BOOST        - list of Boost component names the project needs
#   NO_VERSION_INFO   - set to skip build-info / version embedding
#
# Macros provided (call order):
#   commonStart()          - call first; detects platform, sets up lists
#   addQT()                - find and link Qt6
#   addSourceFiles(...)    - collect source files into the project
#   programEnd()           - finalize an EXE target
#   libraryEnd(SHARED|STATIC) - finalize a library target

set(XQBUILDSYS_PATH ${CMAKE_CURRENT_LIST_DIR})
set(extLibFolder "rdframer_extlib")

# ---------------------------------------------------------------------------
# Platform detection
# ---------------------------------------------------------------------------
macro(setPlatformName)
    if(APPLE)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(ourPlatform macos-${CMAKE_SYSTEM_PROCESSOR})
        else()
            set(ourPlatform macos-x32)
        endif()
        set(ourLibSuffix lib)

    elseif(UNIX)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(ourPlatform linux-${CMAKE_SYSTEM_PROCESSOR})
        else()
            set(ourPlatform linux-x32)
        endif()
        set(ourLibSuffix lib)

    elseif(WIN32)
        set(ourPlatform win32_build)
        set(ourLibSuffix lib_win32_build)

    else()
        message(FATAL_ERROR "Unsupported platform")
    endif()

    message(STATUS "[${PROJECT_NAME}]: platform=${ourPlatform}  output=${ourLibSuffix}")
endmacro()

# ---------------------------------------------------------------------------
# List helpers
# ---------------------------------------------------------------------------
macro(sub_list list1 list2)
    foreach(list2_item ${${list2}})
        list(FIND ${list1} ${list2_item} find_result)
        if(NOT ${find_result} EQUAL -1)
            list(REMOVE_AT ${list1} find_result)
        endif()
        list(APPEND ${list2} ${${list1}})
        list(REMOVE_DUPLICATES ${list2})
    endforeach()
endmacro()

function(my_add_subdirectory _dir)
    get_filename_component(_fullpath "${_dir}" REALPATH)
    if(EXISTS "${_fullpath}" AND EXISTS "${_fullpath}/CMakeLists.txt")
        get_property(_included_dirs GLOBAL PROPERTY GlobalAddSubdirectoryOnceIncluded)
        list(FIND _included_dirs "${_fullpath}" _used_index)
        if(${_used_index} EQUAL -1)
            set_property(GLOBAL APPEND PROPERTY GlobalAddSubdirectoryOnceIncluded "${_fullpath}")
            add_subdirectory(${_dir} ${ARGN})
        endif()
    else()
        message(WARNING "my_add_subdirectory: Can't find ${_fullpath}/CMakeLists.txt")
    endif()
endfunction()

# ---------------------------------------------------------------------------
# Boost
# ---------------------------------------------------------------------------
macro(addBoost)
    set(Boost_USE_STATIC_LIBS OFF)
    set(Boost_USE_MULTITHREADED ON)
    set(Boost_NO_WARN_NEW_VERSIONS ON)

    if(DEFINED ENV{BOOST_ROOT})
        set(BOOST_ROOT $ENV{BOOST_ROOT})
    endif()
    if(DEFINED ENV{BOOST_LIBRARYDIR})
        set(BOOST_LIBRARYDIR $ENV{BOOST_LIBRARYDIR})
    endif()

    add_definitions(-DBOOST_ALL_DYN_LINK -DUSING_BOOST_LIBRARY)

    # Filter out 'system' - it's header-only since Boost 1.69
    set(_boost_components ${ARGN})
    list(REMOVE_ITEM _boost_components "system")
    message(STATUS "[${PROJECT_NAME}]: adding Boost components: ${_boost_components}")

    if(NOT OUR_BOOST_COMPONENTS_LIST)
        set(OUR_BOOST_COMPONENTS_LIST ${_boost_components})
        find_package(Boost REQUIRED COMPONENTS ${OUR_BOOST_COMPONENTS_LIST})
        set(OUR_BOOST_LIBRARIES_LIST ${Boost_LIBRARIES})
        set(OUR_BOOST_INCLUDE_LIST ${Boost_INCLUDE_DIRS})
    else()
        set(tmplist1 ${ARGN})
        sub_list(tmplist1 OUR_BOOST_COMPONENTS_LIST)
        if(tmplist1)
            message(STATUS "[${PROJECT_NAME}]: new Boost components: ${tmplist1}")
            set(OUR_BOOST_COMPONENTS_LIST ${OUR_BOOST_COMPONENTS_LIST} PARENT_SCOPE)
            find_package(Boost REQUIRED COMPONENTS ${tmplist1})
            list(APPEND OUR_BOOST_INCLUDE_LIST ${Boost_INCLUDE_DIRS})
            list(APPEND OUR_BOOST_LIBRARIES_LIST ${Boost_LIBRARIES})
            list(REMOVE_DUPLICATES OUR_BOOST_INCLUDE_LIST)
            list(REMOVE_DUPLICATES OUR_BOOST_LIBRARIES_LIST)
            set(OUR_BOOST_INCLUDE_LIST ${OUR_BOOST_INCLUDE_LIST} PARENT_SCOPE)
            set(OUR_BOOST_LIBRARIES_LIST ${OUR_BOOST_LIBRARIES_LIST} PARENT_SCOPE)
        endif()
    endif()

    list(APPEND INC_DIR_LIST ${OUR_BOOST_INCLUDE_LIST})
    list(APPEND ${L_LIST} ${OUR_BOOST_LIBRARIES_LIST})
    message(STATUS "[${PROJECT_NAME}]: Boost done: ${OUR_BOOST_COMPONENTS_LIST}")
endmacro()

# ---------------------------------------------------------------------------
# Qt6
# ---------------------------------------------------------------------------
macro(addQT)
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)

    if(DEFINED ENV{QTDIR})
        list(APPEND CMAKE_PREFIX_PATH $ENV{QTDIR})
    endif()
    if(APPLE)
        # Homebrew Qt6 (the unversioned "qt" formula is Qt6)
        foreach(_prefix "/opt/homebrew/opt/qt" "/usr/local/opt/qt")
            if(EXISTS "${_prefix}")
                list(APPEND CMAKE_PREFIX_PATH "${_prefix}")
            endif()
        endforeach()
    endif()

    if(NOT QT6_LIBRARIES_MINE)
        message(STATUS "[${PROJECT_NAME}]: finding Qt6 (platform ${ourPlatform})")
        find_package(Qt6 REQUIRED COMPONENTS Core Gui OpenGL OpenGLWidgets Widgets Network)
        set(QT6_LIBRARIES_MINE YES)
    endif()

    list(APPEND ${L_LIST} Qt6::Core Qt6::Widgets Qt6::Gui Qt6::Network Qt6::OpenGL Qt6::OpenGLWidgets)
    set(AUTOGEN_TARGETS_FOLDER automoc)
    set(now_using_QT YES)
    set(now_using_QT6 YES)

    cmake_parse_arguments(earg "" "USE_CMD" "" ${ARGN})
    if(earg_USE_CMD)
        set(QT_KEEP_CMD ON)
    endif()
endmacro()

# ---------------------------------------------------------------------------
# Qwt  (vendored submodule at extern/qwt, built as static lib by extern/CMakeLists.txt)
# ---------------------------------------------------------------------------
macro(addQWT)
    list(APPEND ${L_LIST} qwt)
    message(STATUS "[${PROJECT_NAME}]: using vendored Qwt (extern/qwt)")
endmacro()

# Keep legacy name as alias
macro(addQWTLinux)
    addQWT()
endmacro()

# ---------------------------------------------------------------------------
# Output directory helpers
# ---------------------------------------------------------------------------
macro(findOurLibs)
    if(NOT OUR_LIBRARY_DIR)
        find_file(OUR_LIBRARY_DIR ${ourLibSuffix}
            PATHS ../ ../../ ../../../ ../../../../ NO_DEFAULT_PATH)
        if(NOT OUR_LIBRARY_DIR)
            find_path(buildsys_folder "buildsys"
                PATHS ../ ../../ ../../../ ../../../../ ../../../../../ NO_DEFAULT_PATH)
            if(NOT buildsys_folder)
                find_path(git_root_folder ".git"
                    PATHS ../ ../../ ../../../ ../../../../ ../../../../../ ../../../../../../ NO_DEFAULT_PATH)
                if(NOT git_root_folder)
                    set(OUR_LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${ourLibSuffix}")
                else()
                    set(OUR_LIBRARY_DIR "${git_root_folder}/${ourLibSuffix}")
                endif()
            else()
                set(bf "${buildsys_folder}")
                find_path(git_root_folder ".git"
                    PATHS ${bf}/../ ${bf}/../../ ${bf}/../../../ ${bf}/../../../../ NO_DEFAULT_PATH)
                if(NOT git_root_folder)
                    set(OUR_LIBRARY_DIR ${bf}/${ourLibSuffix})
                else()
                    set(OUR_LIBRARY_DIR "${git_root_folder}/${ourLibSuffix}")
                endif()
            endif()

            if(NOT EXISTS ${OUR_LIBRARY_DIR})
                get_filename_component(OUR_LIBRARY_DIR ${OUR_LIBRARY_DIR} ABSOLUTE
                    BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
                message(STATUS "creating ${OUR_LIBRARY_DIR}")
                file(MAKE_DIRECTORY ${OUR_LIBRARY_DIR})
            endif()
            if(NOT EXISTS ${OUR_LIBRARY_DIR})
                message(FATAL_ERROR "${PROJECT_NAME}: cannot create ${OUR_LIBRARY_DIR}")
            endif()
        endif()
    endif()

    # Platform-specific output subdirectory so macOS and Linux builds
    # don't overwrite each other on shared filesystems (e.g. OneDrive).
    set(OUR_LIBRARY_DIR "${OUR_LIBRARY_DIR}/${ourPlatform}")
    if(NOT EXISTS "${OUR_LIBRARY_DIR}")
        file(MAKE_DIRECTORY "${OUR_LIBRARY_DIR}")
    endif()

    message(STATUS "[${PROJECT_NAME}]: libs dir = ${OUR_LIBRARY_DIR}")
endmacro()

# ---------------------------------------------------------------------------
# Version / build-number embedding
# ---------------------------------------------------------------------------
macro(addVersionInfo curSrcList)
    find_file(VERSION_INFO_FILE version.txt
        PATHS . ../ ../../ ../../../ ../../../../ NO_DEFAULT_PATH)
    find_file(BUILDINFO_PY_SCRIPT "build_info.py"
        PATHS "${XQBUILDSYS_PATH}" NO_DEFAULT_PATH)

    if(NOT VERSION_INFO_FILE)
        file(WRITE "../version.txt" "1.0")
        find_file(VERSION_INFO_FILE version.txt
            PATHS . ../ ../../ ../../../ ../../../../ NO_DEFAULT_PATH)
        if(NOT VERSION_INFO_FILE)
            message(FATAL_ERROR "cannot find version.txt")
        endif()
    endif()
    if(NOT BUILDINFO_PY_SCRIPT)
        message(FATAL_ERROR "cannot find build_info.py (XQBUILDSYS_PATH=${XQBUILDSYS_PATH})")
    endif()

    set(BUILD_INFO_FILE "${CMAKE_CURRENT_BINARY_DIR}/build_info_${PROJECT_NAME}.cpp")
    list(APPEND ${curSrcList} ${BUILD_INFO_FILE})

    set(BN_FILE "${CMAKE_CURRENT_BINARY_DIR}/build_number_${PROJECT_NAME}.cpp")
    list(APPEND ${curSrcList} ${BN_FILE})
    if(NOT EXISTS ${BN_FILE})
        file(WRITE ${BN_FILE} "0")
    endif()

    add_custom_command(OUTPUT ${BUILD_INFO_FILE}
        COMMAND python3 ${BUILDINFO_PY_SCRIPT} ${VERSION_INFO_FILE} ${BUILD_INFO_FILE} ${ourPlatform}
        DEPENDS ${VERSION_INFO_FILE}
        COMMENT "Generating ${BUILD_INFO_FILE}")

    file(STRINGS ${VERSION_INFO_FILE} VERSION_INFO_STRING)
    message(STATUS "version: ${VERSION_INFO_STRING}")
endmacro()

macro(addBuildNumber)
    set(_bn_search_name "buildnumber_${PROJECT_NAME}.txt")
    find_file(BN_INFO_FILE ${_bn_search_name}
        PATHS ${CMAKE_CURRENT_BINARY_DIR} NO_DEFAULT_PATH)
    find_file(BN_PY_SCRIPT incbuildnumber.py
        PATHS ${XQBUILDSYS_PATH} NO_DEFAULT_PATH)

    if(NOT BN_PY_SCRIPT)
        message(FATAL_ERROR "cannot find incbuildnumber.py (XQBUILDSYS_PATH=${XQBUILDSYS_PATH})")
    endif()

    if(NOT BN_INFO_FILE)
        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${_bn_search_name}" "1")
        find_file(BN_INFO_FILE ${_bn_search_name}
            PATHS ${CMAKE_CURRENT_BINARY_DIR} NO_DEFAULT_PATH)
        if(NOT BN_INFO_FILE)
            message(FATAL_ERROR "cannot create ${_bn_search_name}")
        endif()
    endif()

    set(BN_FILE ${CMAKE_CURRENT_BINARY_DIR}/build_number_${PROJECT_NAME}.cpp)

    find_path(extLibFolder1 ${extLibFolder}
        PATHS ../ ../../ ../../../ ../../../../ ../../../../../ ../../../../../../ NO_DEFAULT_PATH)
    if(NOT extLibFolder1)
        set(extLibFolder1 "")
    else()
        set(extLibFolder1 "${extLibFolder1}/${extLibFolder}")
    endif()

    add_custom_target(INC_BN_${PROJECT_NAME} ALL
        python3 ${BN_PY_SCRIPT} ${BN_INFO_FILE} ${BN_FILE}
            ${CMAKE_CURRENT_SOURCE_DIR} ${XQBUILDSYS_PATH} ${extLibFolder1}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Incrementing build number for ${PROJECT_NAME}" VERBATIM)

    if(NOT PROJECTS_LIST)
        add_dependencies(${PROJECT_NAME} INC_BN_${PROJECT_NAME})
    endif()
endmacro()

# ---------------------------------------------------------------------------
# External library helpers  (kept for compatibility but not actively used)
# ---------------------------------------------------------------------------
macro(addExtLibrary libNickname)
    unset(EXT_LIBRARY_DIR CACHE)
    set(EXT_LIBRARY_DIR EXT_LIBRARY_DIR-NOTFOUND)
    find_path(EXT_LIBRARY_DIR ${extLibFolder}
        PATHS ../ ../../ ../../../ ../../../../ ../../../../../ ../../../../../../ NO_DEFAULT_PATH)
    if(NOT EXT_LIBRARY_DIR)
        message(FATAL_ERROR "addExtLibrary: cannot find '${extLibFolder}'")
    else()
        set(EXT_LIBRARY_DIR "${EXT_LIBRARY_DIR}/${extLibFolder}")
    endif()

    cmake_parse_arguments(earg "" "VSTRING;RLNAME;DLNAME" "" ${ARGN})
    set(extLibPath ${EXT_LIBRARY_DIR}/${libNickname})
    if(earg_VSTRING)
        set(extLibPath ${extLibPath}/${earg_VSTRING})
    endif()
    if(NOT EXISTS ${extLibPath})
        message(FATAL_ERROR "addExtLibrary(${libNickname}): cannot find ${extLibPath}")
    endif()
    set(extLibPlatformPath ${extLibPath}/${ourPlatform})

    if(earg_RLNAME)
        set(lName "${extLibPlatformPath}/lib/${earg_RLNAME}${CMAKE_LINK_LIBRARY_SUFFIX}")
        if(NOT EXISTS ${lName})
            message(FATAL_ERROR "addExtLibrary: cannot locate ${lName}")
        endif()
        list(APPEND ${L_LIST} ${lName})
    endif()

    list(APPEND INC_DIR_LIST "${extLibPath}/include")
    file(GLOB incSubDirs LIST_DIRECTORIES true "${extLibPath}/include/*")
    foreach(incSubDir ${incSubDirs})
        if(IS_DIRECTORY ${incSubDir})
            list(APPEND INC_DIR_LIST ${incSubDir})
        endif()
    endforeach()
endmacro()

macro(addOtherLib)
    list(APPEND ${L_LIST} ${ARGN})
endmacro()

# ---------------------------------------------------------------------------
# Source file collection
# ---------------------------------------------------------------------------
macro(addSourceFiles groupName sourceFilesPath)
    get_filename_component(abs_file_path ${sourceFilesPath} ABSOLUTE
        BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    set(extList "cpp" "cc" "c" "h" "hpp" "hh")
    set(sFileList)
    list(APPEND INC_DIR_LIST ${abs_file_path})

    foreach(sfName ${ARGN})
        set(currentFileList)
        foreach(ext ${extList})
            set(aFile aFile-NOTFOUND)
            find_file(aFile "${sfName}.${ext}" PATHS ${abs_file_path} NO_DEFAULT_PATH)
            if(aFile)
                list(APPEND currentFileList ${aFile})
            endif()
            set(aFile aFile-NOTFOUND)
        endforeach()
        if(NOT currentFileList)
            # try exact name (e.g. .qrc files)
            find_file(aFile ${sfName} PATHS ${abs_file_path} NO_DEFAULT_PATH)
            if(aFile)
                list(APPEND currentFileList ${aFile})
            endif()
            set(aFile aFile-NOTFOUND)
        endif()
        if(NOT currentFileList)
            message(FATAL_ERROR "addSourceFiles: cannot find '${sfName}' in ${abs_file_path}")
        else()
            list(APPEND ${PROJ_SRC_FILES} ${currentFileList})
            list(APPEND sFileList ${currentFileList})
        endif()
    endforeach()

    if(sFileList)
        set(group_${groupName}_files ${sFileList})
        list(APPEND ourGroupNames ${groupName})
    endif()
endmacro()

macro(addSourceFilesEverything groupName sourceFilesPath)
    set(extList ".cpp" ".cc" ".c" ".h" ".hpp" ".hh")
    set(eList)
    list(APPEND INC_DIR_LIST ${sourceFilesPath})
    foreach(e1 ${extList})
        list(APPEND eList "${sourceFilesPath}/*${e1}")
    endforeach()
    file(GLOB sFileList LIST_DIRECTORIES false ${eList})
    if(sFileList)
        list(APPEND ${PROJ_SRC_FILES} ${sFileList})
        set(group_${groupName}_files ${sFileList})
        list(APPEND ourGroupNames ${groupName})
    else()
        message(FATAL_ERROR "addSourceFilesEverything: no files in ${sourceFilesPath}")
    endif()
endmacro()

macro(addSourceFilesSimple dstVar groupName sourceFilesPath)
    set(extList "cpp" "cc" "c" "h" "hpp" "hh")
    list(APPEND INC_DIR_LIST ${sourceFilesPath})
    foreach(sfName ${ARGN})
        set(currentFileList)
        foreach(ext ${extList})
            set(aFile aFile-NOTFOUND)
            find_file(aFile "${sfName}.${ext}" PATHS ${sourceFilesPath} NO_DEFAULT_PATH)
            if(aFile)
                list(APPEND currentFileList ${aFile})
            endif()
            set(aFile aFile-NOTFOUND)
        endforeach()
        if(NOT currentFileList)
            message(FATAL_ERROR "addSourceFilesSimple: cannot find '${sfName}' in ${sourceFilesPath}")
        else()
            list(APPEND ${dstVar} ${currentFileList})
        endif()
    endforeach()
endmacro()

# ---------------------------------------------------------------------------
# commonStart  -  call BEFORE adding files
# ---------------------------------------------------------------------------
macro(commonStart)
    project(${PROJECT_NAME})
    message(STATUS "=== project ${PROJECT_NAME} ===")

    set(CMAKE_COLOR_MAKEFILE TRUE)
    set(CMAKE_VERBOSE_MAKEFILE OFF)

    if(VCPKG_TOOLCHAIN)
        set(VCPKG_EXISTS 1)
    endif()

    setPlatformName()

    if(NOT VCPKG_EXISTS)
        findOurLibs()
    endif()

    set(PROJ_SRC_FILES  SL_${PROJECT_NAME})
    set(L_LIST  LIBS_${PROJECT_NAME})
    set(LR_LIST LIBS_RELEASE_${PROJECT_NAME})
    set(LD_LIST LIBS_DEBUG_${PROJECT_NAME})
    set(OUR_DEPS_LIST)
endmacro()

# ---------------------------------------------------------------------------
# Internal library helpers
# ---------------------------------------------------------------------------
macro(addOurLib)
    if(VCPKG_EXISTS)
        message(FATAL_ERROR "addOurLib not usable with VCPKG")
    endif()
    set(ourRLib ourRLib-NOTFOUND)
    set(ourDLib ourDLib-NOTFOUND)
    foreach(ourLib ${ARGN})
        find_library(ourRLib "${ourLib}" PATHS ${OUR_LIBRARY_DIR}/release NO_DEFAULT_PATH)
        if(NOT ourRLib)
            message(FATAL_ERROR "${PROJECT_NAME}: cannot find ${ourLib} (release) in ${OUR_LIBRARY_DIR}/release")
        endif()
        find_library(ourDLib "${ourLib}d" PATHS ${OUR_LIBRARY_DIR}/debug NO_DEFAULT_PATH)
        if(NOT ourDLib)
            message(FATAL_ERROR "${PROJECT_NAME}: cannot find ${ourLib} (debug)")
        endif()
        list(APPEND ${LR_LIST} optimized ${ourRLib})
        list(APPEND ${LD_LIST} debug ${ourDLib})
        set(ourRLib ourRLib-NOTFOUND)
        set(ourDLib ourDLib-NOTFOUND)
    endforeach()
endmacro()

macro(addOurLib2 way name)
    addOurLib(${name})
    get_filename_component(incDir1 ${way} ABSOLUTE)
    list(APPEND INC_DIR_LIST ${incDir1} ${incDir1}/include)
endmacro()

macro(addOurLib3 way)
    set(OUR_LIBNAME_REPORT "" CACHE INTERNAL "OUR_LIBNAME_REPORT")
    my_add_subdirectory(${way})
    message(STATUS "addOurLib3: way=${way} report=${OUR_LIBNAME_REPORT}")
    if(OUR_LIBNAME_REPORT)
        list(APPEND OUR_DEPS_LIST ${OUR_LIBNAME_REPORT})
        list(APPEND ${L_LIST} ${OUR_LIBNAME_REPORT})
    else()
        list(APPEND ${L_LIST} ${way})
    endif()
    list(APPEND INC_DIR_LIST ${way})
    foreach(id inc include)
        set(cn ${CMAKE_CURRENT_SOURCE_DIR}/${way}/${id})
        if(EXISTS ${cn})
            list(APPEND INC_DIR_LIST ${cn})
        endif()
    endforeach()
endmacro()

macro(fillOurLibList releaseLList debugLList)
    set(ourRLib ourRLib-NOTFOUND)
    set(ourDLib ourDLib-NOTFOUND)
    foreach(ourLib ${ARGN})
        find_library(ourRLib "${ourLib}" PATHS ${OUR_LIBRARY_DIR}/release NO_DEFAULT_PATH)
        find_library(ourDLib "${ourLib}d" PATHS ${OUR_LIBRARY_DIR}/debug NO_DEFAULT_PATH)
        if(ourRLib AND ourDLib)
            list(APPEND ${releaseLList} optimized ${ourRLib})
            list(APPEND ${debugLList} debug ${ourDLib})
        endif()
        set(ourRLib ourRLib-NOTFOUND)
        set(ourDLib ourDLib-NOTFOUND)
    endforeach()
endmacro()

# ---------------------------------------------------------------------------
# Target properties
# ---------------------------------------------------------------------------
macro(setPropsOnTarget tgt)
    if(ourLinkOptions)
        target_link_options(${tgt} BEFORE PUBLIC ${ourLinkOptions})
    endif()

    if(VCPKG_EXISTS)
        set(TGT_PROPS_WHERE)
    else()
        set(TGT_PROPS_WHERE
            ARCHIVE_OUTPUT_DIRECTORY_DEBUG   "${OUR_LIBRARY_DIR}/debug"
            ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${OUR_LIBRARY_DIR}/release"
            LIBRARY_OUTPUT_DIRECTORY_DEBUG   "${OUR_LIBRARY_DIR}/debug"
            LIBRARY_OUTPUT_DIRECTORY_RELEASE "${OUR_LIBRARY_DIR}/release"
            RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUR_LIBRARY_DIR}/release"
            RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${OUR_LIBRARY_DIR}/debug"
        )
    endif()

    set_target_properties(${tgt} PROPERTIES
        DEBUG_OUTPUT_NAME   ${tgt}d
        RELEASE_OUTPUT_NAME ${tgt}
        ${TGT_PROPS_WHERE}
    )

    if(OUR_DEPS_LIST)
        foreach(lib ${OUR_DEPS_LIST})
            add_dependencies(${tgt} ${lib})
        endforeach()
    endif()

    if(now_using_QT6)
        set_target_properties(${tgt} PROPERTIES QT_NO_LINK_QTMAIN ON)
    endif()
endmacro()

# ---------------------------------------------------------------------------
# commonEnd  -  finalize target
# ---------------------------------------------------------------------------
macro(commonEnd libType)
    if("${libType}" STREQUAL "SHARED")
        set(BUILD_SHARED_LIBS ON)
    elseif("${libType}" STREQUAL "STATIC")
        set(BUILD_SHARED_LIBS OFF)
    endif()

    list(APPEND INC_DIR_LIST ".")

    add_definitions(-DNOMINMAX -DXQBUILDSYSTEM -DOURPROJECTNAME=${PROJECT_NAME})

    if(WIN32)
        add_definitions(-DWIN32 -DWINVER=0x0A00 -D_WIN32_WINNT=0x0A00)
    endif()

    add_compile_options("$<$<CONFIG:DEBUG>:-DDEBUG123>")

    if(NO_VERSION_INFO)
        message(STATUS "[${PROJECT_NAME}]: no version info (faster build)")
    else()
        addVersionInfo(${PROJ_SRC_FILES})
        add_definitions(-DWITH_VERSION_INFO)
    endif()

    # C++ standard - C++17 by default for broad compatibility (Qwt 6.1 has C++20 issues);
    # override with -DCMAKE_CXX_STANDARD=20 if your platform supports it.
    if(NOT DEFINED CMAKE_CXX_STANDARD)
        set(CMAKE_CXX_STANDARD 17)
    endif()
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

    if(UNIX AND NOT APPLE)
        add_definitions(-DLIN_UX -DUNIX)
    elseif(APPLE)
        add_definitions(-DUNIX)
    endif()

    # --- create the target ---
    if("${libType}" STREQUAL "EXE")
        if(DEFINED PROJECTS_LIST)
            set(src_cur_dir ${CMAKE_CURRENT_SOURCE_DIR})
            foreach(simpleProjectName ${PROJECTS_LIST})
                if(EXISTS "${src_cur_dir}/${simpleProjectName}.cc")
                    set(current_src_list "${src_cur_dir}/${simpleProjectName}.cc")
                elseif(EXISTS "${src_cur_dir}/src/${simpleProjectName}.cc")
                    get_filename_component(srcFileName1 "${src_cur_dir}/src/${simpleProjectName}.cc" ABSOLUTE)
                    set(current_src_list ${srcFileName1})
                else()
                    file(GLOB current_src_list
                        ${src_cur_dir}/${simpleProjectName}/*.cc
                        ${src_cur_dir}/${simpleProjectName}/*.cpp
                        ${src_cur_dir}/${simpleProjectName}/*.c)
                endif()
                if(current_src_list)
                    add_executable(${simpleProjectName} ${current_src_list} ${${PROJ_SRC_FILES}})
                    setPropsOnTarget(${simpleProjectName})
                    target_link_libraries(${simpleProjectName} ${${L_LIST}} ${${LD_LIST}} ${${LR_LIST}})
                else()
                    message(WARNING "cannot find sources for ${simpleProjectName}")
                endif()
            endforeach()
        else()
            add_executable(${PROJECT_NAME} ${${PROJ_SRC_FILES}})
        endif()
    else()
        add_library(${PROJECT_NAME} ${libType} ${${PROJ_SRC_FILES}})
        set(OUR_LIBNAME_REPORT ${PROJECT_NAME} PARENT_SCOPE)
    endif()

    # --- post-target setup ---
    if(NOT NO_VERSION_INFO)
        addBuildNumber()
    endif()

    if(NEED_BOOST)
        addBoost(${NEED_BOOST})
    endif()

    include_directories(${INC_DIR_LIST})

    if(WIN32 AND MSVC)
        add_definitions(/MP /GS)
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /fp:fast /O2 /Ob2 /Oi /Ot /MD")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    endif()

    foreach(ourGroup ${ourGroupNames})
        source_group(${ourGroup} FILES ${group_${ourGroup}_files})
    endforeach()

    if(OUR_LIB_LIST)
        fillOurLibList(${LR_LIST} ${LD_LIST} ${OUR_LIB_LIST})
    endif()

    if(NOT PROJECTS_LIST)
        setPropsOnTarget(${PROJECT_NAME})
        if(NOT "${libType}" STREQUAL "STATIC")
            target_link_libraries(${PROJECT_NAME} ${${L_LIST}} ${${LD_LIST}} ${${LR_LIST}})
        endif()
    endif()

    if(UNIX)
        if("${libType}" STREQUAL "SHARED")
            install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)
        endif()
    endif()
endmacro()

macro(copy_pb whereToCopy)
    if(NOT DEFINED PROJECTS_LIST)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
                $<TARGET_FILE:${PROJECT_NAME}>
                ${whereToCopy}/$<TARGET_FILE_NAME:${PROJECT_NAME}>)
    endif()
endmacro()

macro(programEnd)
    commonEnd(EXE)
endmacro()

macro(libraryEnd libType)
    commonEnd(${libType})
endmacro()
