
# A set of useful functions for cmake
#    \file ccmake.cmake
#    \author   Igor Sandler
#
#    \version 1.0
#
#


# 	NEED_BOOST - list of boost packages that we need
#    ourPlatform  - destination platform ID
#    ourLibSuffix  -  where to put everything (like install-prefix)
# NO_VERSION_INFO  if do not need version info build in 

# macro (copy_pb whereToCopy)  just copy our target somewhere as a post build step
#


cmake_policy(SET CMP0057 NEW)
cmake_policy(SET CMP0071 NEW)
#cmake_policy(SET CMP0074 NEW)
#cmake_policy(VERSION 3.16)
cmake_policy(SET CMP0020 NEW)

set(XQBUILDSYS_PATH ${CMAKE_CURRENT_LIST_DIR})
set(extLibFolder "rdframer_extlib")

macro (setPlatformName)
	include(CMakeParseArguments)
	if (UNIX)
		if (CMAKE_SIZEOF_VOID_P EQUAL 8) 
			set(ourPlatform linux-x64)
			set (ourLibSuffix lib)
		else()
			set(ourPlatform linux-x32)
			set (ourLibSuffix lib-x32)
		endif()
		
	elseif(WIN32)
		message(STATUS "MSVC_VERSION = " ${MSVC_VERSION})
		message(STATUS "MSVC_CXX_ARCHITECTURE_ID = ${MSVC_CXX_ARCHITECTURE_ID}")
		message(STATUS "CMAKE_CL_64 = ${CMAKE_CL_64}")
		set(vs2017_versions "1910" "1911" "1912" "1915" "1916")
		set(vs2019_versions "1920" "1921" "1922" "1923" "1924" "1925" "1926")
	
		if (MSVC_VERSION STREQUAL 1600) # VS 2010 
			if (CMAKE_CL_64) # Using the 64 bit compiler from Microsoft
				set(ourPlatform vs2010-x64)
				set (ourLibSuffix lib-vs2010-x64)
			else() 
				set(ourPlatform vs2010-x32)
				set (ourLibSuffix lib-vs2010-x32)
			endif()
		elseif(MSVC_VERSION STREQUAL 1800) # VS2013 
			if (CMAKE_CL_64) # Using the 64 bit compiler from Microsoft
				set(ourPlatform vs2013-x64)
				set (ourLibSuffix lib-vs2013-x64)
			else()
				set(ourPlatform vs2013-x32)
				set (ourLibSuffix lib-vs2013-x32)
			endif()
		elseif(MSVC_VERSION STREQUAL 1900) # VS2015 
			if (CMAKE_CL_64) # Using the 64 bit compiler from Microsoft
				set(ourPlatform vs2015-x64)
				set (ourLibSuffix lib-vs2015-x64)
			else()
				set(ourPlatform vs2015-x32)
				set (ourLibSuffix lib-vs2015-x32)
			endif()
		elseif(MSVC_VERSION IN_LIST vs2017_versions) # VS2017
			if (CMAKE_CL_64) # Using the 64 bit compiler from Microsoft
				set(ourPlatform vs2017-x64)
				set (ourLibSuffix lib-vs2017-x64)
			else()
				set(ourPlatform vs2017-x32)
				set (ourLibSuffix lib-vs2017-x32)
			endif()
		elseif(MSVC_VERSION IN_LIST vs2019_versions) # VS2019
			if (CMAKE_CL_64) # Using the 64 bit compiler from Microsoft
				set(ourPlatform vs2019-x64)
				set (ourLibSuffix lib-vs2019-x64)
			else()
				set(ourPlatform vs2019-x32)
				set (ourLibSuffix lib-vs2019-x32)
			endif()


		else()
			message(FATAL_ERROR " this Visual Studio version ${MSVC_VERSION} look like not supported yet")
		endif()
	else()
		message ( FATAL_ERROR " platform not supported " )
	endif()
	
	# look for buildsys folder:
	#find_path(OUR_BUILDFOLDER buildsys PATHS ../ ../../ ../../../ ../../../../  NO_DEFAULT_PATH)
	#if (NOT OUR_BUILDFOLDER)
	#	message(STATUS "[${PROJECT_NAME}]:  warning: can not find buildsys directory")
	#else()
	#	message(STATUS "[${PROJECT_NAME}]:  found buildsys in ${OUR_BUILDFOLDER}")	
	#endif()
	message(STATUS  "[${PROJECT_NAME}]:  detected platform: " ${ourPlatform}; " output dir:  " ${ourLibSuffix})
endmacro()



# substruct list2 from list1
# on the output, list1 = list1 - list2;    and after that list2 = list2 + list1
#
macro (sub_list list1 list2)
	#message(STATUS "[${PROJECT_NAME}]: sub_list begin. (list1 =  ${${list1}}    list2 = ${${list2}};     argn = ${ARGN}")
	foreach (list2_item ${${list2}})
		list(FIND ${list1} ${list2_item} find_result)
		if (${find_result} EQUAL -1)
			#message(STATUS "[${PROJECT_NAME}]: sub_list: got new item ( ${list2_item} ) in 'list1' ")
		else()
			#message(STATUS "[${PROJECT_NAME}]: sub_list: removing item ( ${list2_item} ) from 'list1' ")
			list(REMOVE_AT  ${list1} find_result)
		endif()
		list(APPEND ${list2} ${${list1}})
		list(REMOVE_DUPLICATES ${list2})
	endforeach()
	#message(STATUS "[${PROJECT_NAME}]: sub_list end. (list1 =  ${${list1}}    list2 = ${${list2}};     argn = ${ARGN}")
endmacro()

#
#  from https://stackoverflow.com/users/4763489/florian
#  https://stackoverflow.com/questions/47674915/cmake-check-if-a-higher-level-directory-exists
function(my_add_subdirectory _dir)
    get_filename_component(_fullpath "${_dir}" REALPATH)
    if (EXISTS "${_fullpath}" AND EXISTS "${_fullpath}/CMakeLists.txt")
        get_property(_included_dirs GLOBAL PROPERTY GlobalAddSubdirectoryOnceIncluded)
        list(FIND _included_dirs "${_fullpath}" _used_index)
        if (${_used_index} EQUAL -1)
            set_property(GLOBAL APPEND PROPERTY GlobalAddSubdirectoryOnceIncluded "${_fullpath}")
            add_subdirectory(${_dir} ${ARGN})
        endif()
    else()
        message(WARNING "my_add_subdirectory: Can't find ${_fullpath}/CMakeLists.txt")
    endif()
endfunction(my_add_subdirectory)

# add boost libs and headers
macro (addBoost)	
	#cmake_policy(SET CMP0074 NEW)
	set(Boost_USE_STATIC_LIBS OFF) 
	set(Boost_USE_MULTITHREADED ON)  
	set (BOOST_ROOT $ENV{BOOST_ROOT})
	set (BOOST_LIBRARYDIR $ENV{BOOST_LIBRARYDIR})
	add_definitions(-DBOOST_ALL_DYN_LINK)
	add_definitions(-DUSING_BOOST_LIBRARY)
	message(STATUS  "[${PROJECT_NAME}]: adding BOOST: root = " ${BOOST_ROOT} ; " lib dir = " ${BOOST_LIBRARYDIR} )
	
	if (NOT OUR_BOOST_COMPONENTS_LIST)
		set (OUR_BOOST_COMPONENTS_LIST ${ARGN})
		find_package(Boost ${boost_version} REQUIRED COMPONENTS ${OUR_BOOST_COMPONENTS_LIST})
		set (OUR_BOOST_LIBRARIES_LIST ${Boost_LIBRARIES})
		set (OUR_BOOST_INCLUDE_LIST ${Boost_INCLUDE_DIRS})
	else()
		#message(STATUS "[${PROJECT_NAME}]: add-boost-begin: OUR_BOOST_COMPONENTS_LIST = ${OUR_BOOST_COMPONENTS_LIST}")
		set (tmplist1 ${ARGN})
		#message(STATUS "[${PROJECT_NAME}]: addBoost: (list1 =  ${tmplist1};    list2 = ${OUR_BOOST_COMPONENTS_LIST}")
		sub_list(tmplist1 OUR_BOOST_COMPONENTS_LIST)
		if (tmplist1)
			message(STATUS "[${PROJECT_NAME}]: updating BOOST components list; new components: ${tmplist1}")
			set (OUR_BOOST_COMPONENTS_LIST ${OUR_BOOST_COMPONENTS_LIST} PARENT_SCOPE)
			
			find_package(Boost ${boost_version} REQUIRED COMPONENTS ${tmplist1})
			
			list(APPEND OUR_BOOST_INCLUDE_LIST ${Boost_INCLUDE_DIRS}) 
			list(APPEND OUR_BOOST_LIBRARIES_LIST ${Boost_LIBRARIES})  
			
			list(REMOVE_DUPLICATES OUR_BOOST_INCLUDE_LIST)
			list(REMOVE_DUPLICATES OUR_BOOST_LIBRARIES_LIST)
			
			set (OUR_BOOST_INCLUDE_LIST ${OUR_BOOST_INCLUDE_LIST} PARENT_SCOPE)
			set (OUR_BOOST_LIBRARIES_LIST ${OUR_BOOST_LIBRARIES_LIST} PARENT_SCOPE)
		endif()
		
	endif()
	
	list(APPEND INC_DIR_LIST ${OUR_BOOST_INCLUDE_LIST}) 
	list(APPEND ${L_LIST} ${OUR_BOOST_LIBRARIES_LIST})  
	
	#message(STATUS boost libs:  ${Boost_LIBRARIES})
	message(STATUS "[${PROJECT_NAME}]: add-boost-end: OUR_BOOST_COMPONENTS_LIST = ${OUR_BOOST_COMPONENTS_LIST}")
endmacro()

macro (addOSG)
	find_package(OpenSceneGraph REQUIRED COMPONENTS osgDB osgGA osgUtil osgViewer osgAnimation osgText) 
	include_directories(${OPENSCENEGRAPH_INCLUDE_DIRS})
	list(APPEND ${L_LIST} ${OPENSCENEGRAPH_LIBRARIES})
endmacro()

# add QT5 macro
#addQT(USE_CMD YES) will enable command prompt for EXE files
macro (addQT)
	# Find includes in corresponding build directories
	set(CMAKE_INCLUDE_CURRENT_DIR ON)
	# Instruct CMake to run moc automatically when needed.
	set(CMAKE_AUTOMOC ON)
	set(CMAKE_AUTOUIC ON)
	set(CMAKE_AUTORCC ON)
	set (QTDIR $ENV{QTDIR})  #  ? do we need this really?
	list(APPEND CMAKE_PREFIX_PATH ${QTDIR})
	# Find the Qt library
	if (NOT QT5_LIBRARIES_MINE)
		message(STATUS "adding QT5 library for ${PROJECT_NAME} (platform ${ourPlatform})")
		find_package(Qt5 REQUIRED COMPONENTS Core Gui OpenGL Widgets)
		find_package(Qt5NetworkAuth CONFIG REQUIRED)
		#find_package(Qt5Core REQUIRED)
		#find_package(Qt5Gui REQUIRED)
		#find_package(Qt5Widgets REQUIRED)
		set (QT5_LIBRARIES_MINE YES)
	endif()
	list(APPEND ${L_LIST} Qt5::Core Qt5::Widgets Qt5::Gui Qt5::OpenGL Qt5::NetworkAuth Qt5::NetworkAuthPrivate)
	set(AUTOGEN_TARGETS_FOLDER automoc)
	
	set(now_using_QT YES)
	set(now_using_QT5 YES)
	
	cmake_parse_arguments(earg "" "USE_CMD" "" ${ARGN})
	
	if (earg_USE_CMD) 
		#message("******** with CMD")
		set (QT_KEEP_CMD ON)
	else()
		#message("======= without CMD")
	endif()
endmacro()

# add QT4 macro
macro (addQT4)
	# Find includes in corresponding build directories
	
	# Instruct CMake to run moc automatically when needed.
	set(AUTOGEN_TARGETS_FOLDER automoc)
	
	set(CMAKE_AUTOMOC ON)
	set(CMAKE_INCLUDE_CURRENT_DIR ON)
	
	set(CMAKE_AUTOUIC ON)
	set(CMAKE_AUTORCC ON)
	
	set (QTDIR $ENV{QTDIR})  #  ? do we need this really?
	list(APPEND CMAKE_PREFIX_PATH ${QTDIR})
	
	# Find the Qt library
	if (NOT QT4_LIBRARIES_MINE)
		find_package(Qt4 4.8.7 REQUIRED)
		message(STATUS "got QT libs for [${PROJECT_NAME}]  (platform ${ourPlatform}): ${QT_LIBRARIES}  ${QT_QTMAIN_LIBRARY}; files: ${QT_USE_FILE}; defs: ${QT_DEFINITIONS}")
		include(${QT_USE_FILE})
		set (QT4_LIBRARIES_MINE YES)
	endif()
	
	
	add_definitions(${QT_DEFINITIONS})
	list(APPEND ${L_LIST}  ${QT_LIBRARIES}   ${QT_QTMAIN_LIBRARY})
	#list(APPEND ${L_LIST} "Qt4::Core" "Qt4::Widgets" "Qt4::Gui")
	
	
	
	set(now_using_QT YES)
	set(now_using_QT4 YES)
endmacro()


macro (addQWTLinux) 
	FIND_PATH(QWT_INCLUDE_DIR NAMES qwt.h PATHS  
		/usr/include/qt5/qwt
		/usr/include
		/usr/include/qt5
		"ENV{INCLUDE}"
		PATH_SUFFIXES qwt-qt5 qwt
	)
	FIND_LIBRARY(QWT_LIBRARY NAMES qwt-qt5 qwt PATHS
		/usr/lib64
		/user/local/lib64
		/usr/lib
		/usr/local/lib
		"$ENV{LIB}"
	)
	IF (QWT_INCLUDE_DIR AND QWT_LIBRARY)
		SET(QWT_FOUND TRUE)
		list(APPEND ${L_LIST} ${QWT_LIBRARY})
		list(APPEND INC_DIR_LIST ${QWT_INCLUDE_DIR})

	else()
		message(FATAL_ERROR "cannot find QWT library")
	ENDIF()

	
endmacro()


macro (findOurLibs)
	if (NOT OUR_LIBRARY_DIR)
		find_file(OUR_LIBRARY_DIR ${ourLibSuffix} PATHS ../ ../../ ../../../ ../../../../  NO_DEFAULT_PATH)
		if (NOT OUR_LIBRARY_DIR)# lets create a new one; where?
			find_path(buildsys_folder "buildsys" PATHS ../ ../../ ../../../ ../../../../ ../../../../../  NO_DEFAULT_PATH)
			if (NOT buildsys_folder) # inside a 'normal' repo
				find_path (git_root_folder ".git" PATHS ../ ../../ ../../../ ../../../../ ../../../../../ ../../../../../../  NO_DEFAULT_PATH)
				if (NOT git_root_folder) # this is not even a git repo
					set (OUR_LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${ourLibSuffix}")
				else()
					set (OUR_LIBRARY_DIR "${git_root_folder}/${ourLibSuffix}")
				endif()
			else()
				set(bf "${buildsys_folder}")
				find_path(git_root_folder ".git" PATHS ${bf}/../ ${bf}/../../ ${bf}/../../../ ${bf}/../../../../ ${bf}/../../../../../ ${bf}/../../../../../../  NO_DEFAULT_PATH)
				if (NOT git_root_folder)
					set(OUR_LIBRARY_DIR ${bf}/${ourLibSuffix})
				else()
					set (OUR_LIBRARY_DIR "${git_root_folder}/${ourLibSuffix}")
				endif()
			endif()
		
			IF (NOT EXISTS ${OUR_LIBRARY_DIR})
				if (NOT VCPKG_EXISTS)
					file(MAKE_DIRECTORY ${OUR_LIBRARY_DIR})
				endif()
			endif()
			IF (NOT EXISTS ${OUR_LIBRARY_DIR})
				message(FATAL_ERROR "project ${PROJECT_NAME}: cannot create   ${OUR_LIBRARY_DIR} folder")
			endif()
		endif()
	endif()	
	message(STATUS "project ${PROJECT_NAME}: our libs are in " ${OUR_LIBRARY_DIR})
endmacro()

# add build information to the source files list (curSrcList)
macro (addVersionInfo curSrcList)
	#message(STATUS "XQBUILDSYS_PATH = ${XQBUILDSYS_PATH}")
	#message(STATUS "curSrcList = ${curSrcList}")
	
	
	find_file(VERSION_INFO_FILE version.txt PATHS . ../ ../../ ../../../ ../../../../  NO_DEFAULT_PATH) # where is version.txt file?
	find_file(BUILDINFO_PY_SCRIPT "build_info.py" PATHS "${XQBUILDSYS_PATH}"  NO_DEFAULT_PATH)
	if (NOT VERSION_INFO_FILE) # at first, try to create one:
		file(WRITE "../version.txt" "1.0")
		find_file(VERSION_INFO_FILE version.txt PATHS . ../ ../../ ../../../  ../../../../ NO_DEFAULT_PATH)
		if (NOT VERSION_INFO_FILE) 
			message(FATAL_ERROR "can not find version info file (version.txt); CMAKE_CURRENT_SOURCE_DIR=" ${CMAKE_CURRENT_SOURCE_DIR})
		endif()
	else()
		#logMessage("got version file:  ${VERSION_INFO_FILE}")
		message(STATUS  "got version file:  ${VERSION_INFO_FILE}")
	endif()
	if (NOT BUILDINFO_PY_SCRIPT) 
		message(FATAL_ERROR "can not find BUILDINFO_PY_SCRIPT (XQBUILDSYS_PATH = ${XQBUILDSYS_PATH}")
	endif()
	
	
	set(BUILD_INFO_FILE "${CMAKE_BINARY_DIR}/build_info.cpp") # We are going to create this file after version.txt will change, at compile time
	list(APPEND ${curSrcList} ${BUILD_INFO_FILE}) # append 'build_info.cpp' to the project
	
	set(BN_FILE "${CMAKE_BINARY_DIR}/build_number.cpp") 
	list(APPEND ${curSrcList} ${BN_FILE}) # append 'build_number.cpp' to the project
	if (NOT EXISTS ${BN_FILE})  # we need to create something, or project will not be created
		file(WRITE ${BN_FILE}  "0")
	endif()
	
	# recreate build info:
	add_custom_command(OUTPUT ${BUILD_INFO_FILE} COMMAND python ${BUILDINFO_PY_SCRIPT} ${VERSION_INFO_FILE} ${BUILD_INFO_FILE} ${ourPlatform} DEPENDS ${VERSION_INFO_FILE} COMMENT "creating ${BUILD_INFO_FILE}")

	file(STRINGS  ${VERSION_INFO_FILE} VERSION_INFO_STRING)
	message(STATUS "version:   ${VERSION_INFO_STRING}")
	#set(VERSION_INFO_FOR_LINKER "${VERSION_INFO_STRING}")
	#set(VERSION_INFO_FOR_LINKER " /VERSION:${VERSION_INFO_STRING} ")
endmacro ()

macro (addBuildNumber)
	find_file(BN_INFO_FILE buildnumber.txt PATHS ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/../  NO_DEFAULT_PATH) # where is buildnumber.txt file?
	find_file(BN_PY_SCRIPT incbuildnumber.py PATHS ${XQBUILDSYS_PATH}  NO_DEFAULT_PATH)
	if (NOT BN_PY_SCRIPT) # cannot find one useful python script
		message(STATUS "script: ${BN_PY_SCRIPT}")
		message(FATAL_ERROR "ERROR in addBuildNumber: (XQBUILDSYS_PATH = ${XQBUILDSYS_PATH}; cannot find incbuildnumber.py")
	endif()	
		
	if (NOT BN_INFO_FILE)
		message(STATUS " WARNING: cannot find build info file... creating the new one")
		file(WRITE "${CMAKE_BINARY_DIR}/buildnumber.txt" "1")
		find_file(BN_INFO_FILE buildnumber.txt PATHS ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/../  NO_DEFAULT_PATH) 
		if (NOT BN_INFO_FILE)
			message(FATAL_ERROR "ERROR in addBuildNumber: cannot create buildnumber.txt")
		endif()
	endif()
	
	set(BN_FILE ${CMAKE_BINARY_DIR}/build_number.cpp) 
	
	# lets try to obtain a path to the externals git repo; 
	# we need this for incbuildnumber.py script
	find_path(extLibFolder1 ${extLibFolder} PATHS ../ ../../ ../../../ ../../../../ ../../../../../ ../../../../../../   NO_DEFAULT_PATH)
	if (NOT extLibFolder1)
		set(extLibFolder1 "")
	else()
		set(extLibFolder1 "${extLibFolder1}/${extLibFolder}")
	endif()

	
	add_custom_target(INC_BN_${PROJECT_NAME} ALL  python ${BN_PY_SCRIPT} ${BN_INFO_FILE} ${BN_FILE} ${CMAKE_CURRENT_SOURCE_DIR}  ${XQBUILDSYS_PATH} ${extLibFolder1}   WORKING_DIRECTORY  ${CMAKE_BINARY_DIR}  COMMENT "creating new build number" VERBATIM)
	#
	
	message(STATUS "[${PROJECT_NAME}]	 addBuildNumber: script = ${BN_PY_SCRIPT}; files = ${BN_INFO_FILE}, ${BN_FILE}; buildsys folder = ${XQBUILDSYS_PATH}")
	
	if (NOT PROJECTS_LIST)
	add_dependencies(${PROJECT_NAME} INC_BN_${PROJECT_NAME})
	endif()
	
	#add_custom_command(TARGET ${PROJECT_NAME}PRE_BUILD COMMAND  python ARGS ${BN_PY_SCRIPT} ${BN_INFO_FILE} ${BN_FILE} WORKING_DIRECTORY  ${CMAKE_BINARY_DIR} COMMENT "creating new build number" VERBATIM)
endmacro()


# ============= addExtLibrary   macro =========
# params: 
# 1.  library dir
# 2. ".lib" file name (release)   if absent, grab everything
# 3. ".lib" file name (debug) - this is optional 
macro (addExtLibrary libNickname)

	# 1. find ext library dir:
	unset(EXT_LIBRARY_DIR CACHE)
	set(EXT_LIBRARY_DIR EXT_LIBRARY_DIR-NOTFOUND)
	find_path(EXT_LIBRARY_DIR ${extLibFolder} PATHS ../ ../../ ../../../ ../../../../ ../../../../../ ../../../../../../   NO_DEFAULT_PATH)
	if (NOT EXT_LIBRARY_DIR)
		message(FATAL_ERROR "addExtLibrary::error: can not find '${extLibFolder}' directory")
	else()
		set(EXT_LIBRARY_DIR "${EXT_LIBRARY_DIR}/${extLibFolder}")
	endif()
	#message(STATUS "EXT_LIBRARY_DIR =  ${EXT_LIBRARY_DIR} ")
	
	cmake_parse_arguments(earg "" "VSTRING;RLNAME;DLNAME" "" ${ARGN})
	set(extLibPath ${EXT_LIBRARY_DIR}/${libNickname})
	if (earg_VSTRING)   # use version string, if needed
		set(extLibPath ${extLibPath}/${earg_VSTRING})
	endif()
	if (NOT EXISTS ${extLibPath})
		message(FATAL_ERROR "addExtLibrary ( ${libNickname})::error: can not find ${extLibPath} directory")
	endif()
	set(extLibPlatformPath ${extLibPath}/${ourPlatform})
	
	#if RLNAME was provided, 'lName' and 'dName' vars should be OK
	if (earg_RLNAME) # choose particular library with this name:
		message(STATUS "adding external library ${earg_RLNAME}")
		set(lName "${extLibPlatformPath}/lib/${earg_RLNAME}") 
		if (earg_DLNAME) # we have a very special name for debug version
			set(dName "${extLibPlatformPath}/lib/${earg_DLNAME}${CMAKE_LINK_LIBRARY_SUFFIX}") 
			# this dName MUST exist, since we specify it here:
			if (NOT EXISTS "${dName}") 
				message(FATAL_ERROR "addExtLibrary: cannot locate ${dName} file") 
			endif()
		else() # use name for 'release'.d; 
			set(dName "${lName}d${CMAKE_LINK_LIBRARY_SUFFIX}") 
		endif()
		set(lName "${lName}${CMAKE_LINK_LIBRARY_SUFFIX}") # finalize release library name
		if (NOT EXISTS ${lName}) 
			message(FATAL_ERROR "addExtLibrary: cannot locate ${lName} file") 
		endif()
		message(STATUS "	lName = ${lName}")
					
		if (EXISTS ${dName}) # we have got separate debug and release
			list(APPEND ${LR_LIST} optimized ${lName})
			list(APPEND ${LD_LIST} debug ${dName})
			message(STATUS "	${libNickname} added (release ( ${lName} ) and debug  ( ${dName} )  )")
		else()	# everything is in one place
			list(APPEND ${L_LIST} ${lName})
			message(STATUS "	${libNickname} added (release only) from ( ${lName} )")
		endif()
	
	else() # lets grab everything in this case:
		#	set(lName "${extLibPlatformPath}/lib/${libNickname}") 
		#
		#	lets grab all the files:
		#
		if (EXISTS "${extLibPlatformPath}/lib/release")
			set (thisLibFilesRelease)
			file(GLOB thisLibFilesRelease "${extLibPlatformPath}/lib/release/*")
			foreach (item ${thisLibFilesRelease})
				list(APPEND ${LR_LIST} optimized ${item})
			endforeach()
		endif()
		if (EXISTS "${extLibPlatformPath}/lib/debug")
			set (thisLibFilesDebug)
			file(GLOB thisLibFilesDebug "${extLibPlatformPath}/lib/debug/*")
			foreach (item ${thisLibFilesDebug})
				list(APPEND ${LD_LIST} debug ${item})
			endforeach()
		endif()
		
		set(thisLibFiles)
		file(GLOB thisLibFiles "${extLibPlatformPath}/lib/*.lib")
		if (thisLibFiles)
			list(APPEND ${L_LIST} ${thisLibFiles})
		endif()
		
		
		#message(STATUS "following libs were added: ")
		#message(STATUS ${thisLibFiles})
	endif()

	list(APPEND INC_DIR_LIST "${extLibPath}/include")
	
	#lets collect all the subfolders from 'include' here:
	FILE(GLOB incSubDirs LIST_DIRECTORIES true  "${extLibPath}/include/*")
	SET(incSubDirList "")
	FOREACH(incSubDir ${incSubDirs})
		IF(IS_DIRECTORY ${incSubDir})
			LIST(APPEND incSubDirList ${incSubDir})
		ENDIF()
	ENDFOREACH()
	list(APPEND INC_DIR_LIST ${incSubDirList})

endmacro()

# just add some library
macro (addOtherLib)
	list(APPEND ${L_LIST} ${ARGN})
endmacro()

# usage: addSourceFiles(path_to_our_files file1 file2 ...   file_n)
# file names without extension
macro (addSourceFiles groupName sourceFilesPath)
	#GET_FILENAME_COMPONENT(ourGroupName ${sourceFilesPath} NAME)
	#message(STATUS "source group ${ourGroupName} was added")
	set(extList ".cpp" ".cc" ".c" ".h" ".hpp" ".hh")
	set(sFileList)
	list(APPEND INC_DIR_LIST ${sourceFilesPath})
	foreach(sfName ${ARGN})
		set(currentFileList) # clear this list
		foreach(ext ${extList})
			set(aFile aFile-NOTFOUND)
			set(aFileName "${sfName}${ext}")
			#message(STATUS "    looking for ${aFileName} in a ${sourceFilesPath}")
			find_file(aFile ${aFileName} PATHS ${sourceFilesPath} NO_DEFAULT_PATH)
			if(aFile) 
				list(APPEND currentFileList ${aFile})
				#message(STATUS "     file ${aFile} added")
			endif()			
			set(aFile aFile-NOTFOUND)
		endforeach()
		if (NOT currentFileList) # test without extencion
			find_file(aFile ${sfName} PATHS ${sourceFilesPath} NO_DEFAULT_PATH)
			if(aFile) 
				list(APPEND currentFileList ${aFile})
			endif()			
			set(aFile aFile-NOTFOUND)
		endif()
		
		if (NOT currentFileList) 
			message(STATUS "tested following files: ")
			foreach(ext ${extList})
				message(STATUS "	${sfName}${ext}")
			
			endforeach()
			message(FATAL_ERROR "error: in path ${sourceFilesPath} can not add file ${sfName}")
		else()
			list(APPEND ${PROJ_SRC_FILES}  ${currentFileList})
			list(APPEND sFileList  ${currentFileList})
			
			#message(STATUS "file ${sfName}: added as ${currentFileList}")
			#message(STATUS "${PROJ_SRC_FILES} =  ${${PROJ_SRC_FILES}}")
		endif()
		
	endforeach()
	#message(STATUS "adding source_group ${groupName}")
	#source_group(${groupName} FILES ${sFileList})
	
	#set(group_${groupName} ${groupName})
	if (sFileList)
		set(group_${groupName}_files ${sFileList})
		list(APPEND ourGroupNames ${groupName})
	endif()
endmacro()


macro (addSourceFilesEverything groupName sourceFilesPath)
	set(extList ".cpp" ".cc" ".c" ".h" ".hpp" ".hh")
	set(eList)
	list(APPEND INC_DIR_LIST ${sourceFilesPath})
	foreach(e1 ${extList})
		list(APPEND eList "${sourceFilesPath}/*${e1}")
	endforeach()
	file(GLOB sFileList LIST_DIRECTORIES false ${eList})
	
	if (sFileList)
		list(APPEND ${PROJ_SRC_FILES}  ${sFileList})
		set(group_${groupName}_files ${sFileList})
		list(APPEND ourGroupNames ${groupName})
	else()
		message(FATAL_ERROR "error: can not add files from  ${sourceFilesPath}")
	endif()
	
endmacro()


macro (addSourceFilesSimple dstVar groupName sourceFilesPath)
	set(extList "cpp" "cc" "c" "h" "hpp" "hh")
	set(sFileList)
	list(APPEND INC_DIR_LIST ${sourceFilesPath})
	foreach(sfName ${ARGN})
		#message(STATUS "    " ${sfName} ":")
		set(currentFileList) # clear this list
		foreach(ext ${extList})
			set(aFile aFile-NOTFOUND)
			set(aFileName "${sfName}.${ext}")
			#message(STATUS "    looking for ${aFileName} in a ${sourceFilesPath}")
			find_file(aFile ${aFileName} PATHS ${sourceFilesPath} NO_DEFAULT_PATH)
			if(aFile) 
				list(APPEND currentFileList ${aFile})
				#message(STATUS "     file ${aFile} added")
			endif()			
			set(aFile aFile-NOTFOUND)
		endforeach()
		if (NOT currentFileList) 
			message("error: in path ${sourceFilesPath}")
			message(FATAL_ERROR "error: can not add file ${sfName}")
		else()
			list(APPEND ${dstVar}  ${currentFileList})
			#message(STATUS "     files ${currentFileList} added")
		endif()
		
	endforeach()
endmacro()


# call me BEFORE adding files to project
macro (commonStart)
	PROJECT(${PROJECT_NAME})
	message(STATUS "start project ___${PROJECT_NAME}___")
		
	set(CMAKE_COLOR_MAKEFILE TRUE)
	set(CMAKE_VERBOSE_MAKEFILE OFF)
	#if (VCPKG_TOOLCHAIN OR (DEFINED ENV{VCPKG_ROOT}))
	if (VCPKG_TOOLCHAIN)
		set(VCPKG_EXISTS 1)
	endif()
	setPlatformName()
	if (NOT VCPKG_EXISTS) 
		findOurLibs()
	endif()
	
	set(PROJ_SRC_FILES  SL_${PROJECT_NAME}) # list of the all the source files for this project
	set(L_LIST LIBS_${PROJECT_NAME})		# libraries to link to
	set(LR_LIST LIBS_RELEASE_${PROJECT_NAME}) 	# release libraries to link to
	set(LD_LIST LIBS_DEBUG_${PROJECT_NAME})	# debug libraries to link to
	set(OUR_DEPS_LIST)
endmacro()

# add our libraries, which were created with this build system before
# since we supposed to know the place where they are
# TODO: use fillOurLibList macro below
macro (addOurLib)
	if (VCPKG_EXISTS)
		message(FATAL_ERROR "using addOurLib macro together with VCPKG_EXISTS")
	endif()
	set(ourRLib ourRLib-NOTFOUND)
	set(ourDLib ourDLib-NOTFOUND)
	

	foreach(ourLib ${ARGN})
	    find_library(ourRLib "${ourLib}" PATHS ${OUR_LIBRARY_DIR}/release NO_DEFAULT_PATH)
	    if (NOT ourRLib)
	        message(FATAL_ERROR "error: project ${PROJECT_NAME}: can not find ${ourLib} (release) lib in ${OUR_LIBRARY_DIR}/release")
	    endif()
	    find_library(ourDLib "${ourLib}d" PATHS ${OUR_LIBRARY_DIR}/debug NO_DEFAULT_PATH)
	    if (NOT ourDLib)
	        message(FATAL_ERROR "error: can not find ${ourLib} (debug) lib ")
	    endif()
	    #message(STATUS "....adding " ${ourLib} " libraries " ${ourRLib} " and " ${ourDLib})

	    list(APPEND ${LR_LIST}  optimized ${ourRLib})
	    list(APPEND ${LD_LIST}  debug ${ourDLib})
	    
	    set(ourRLib ourRLib-NOTFOUND)
	    set(ourDLib ourDLib-NOTFOUND)
	endforeach()
endmacro()

# strange. Add 'our' library, but add some strange additional include folders
macro (addOurLib2 way name)
	addOurLib(${name})
	get_filename_component(incDir1 ${way} ABSOLUTE) # normalize header file path
	list(APPEND INC_DIR_LIST ${incDir1} ${incDir1}/include) # update include path
endmacro()

# add our library as a dependency
macro (addOurLib3 way)
	set(OUR_LIBNAME_REPORT "" CACHE INTERNAL "OUR_LIBNAME_REPORT")

	my_add_subdirectory(${way})
	

	#add_subdirectory(${way} ${OUR_LIBRARY_DIR})
	#add_subdirectory(${way})
	
	message(STATUS "addOurLib3: way = ${way} OUR_LIBNAME_REPORT = ${OUR_LIBNAME_REPORT}")
	if (OUR_LIBNAME_REPORT)
		list(APPEND OUR_DEPS_LIST ${OUR_LIBNAME_REPORT})
		#list(APPEND ${LR_LIST}  optimized ${OUR_LIBRARY_DIR}/release/${OUR_LIBNAME_REPORT}.lib)
		#list(APPEND ${LD_LIST}  debug ${OUR_LIBRARY_DIR}/debug/${OUR_LIBNAME_REPORT}d.lib)
		list(APPEND ${L_LIST} ${OUR_LIBNAME_REPORT})
	else()
		list(APPEND ${L_LIST} ${way})
	endif()
	
	# add include folders:
	list(APPEND INC_DIR_LIST ${way})
	set(INC5_LIST inc include)
	foreach (id  ${INC5_LIST})
		set(cn ${CMAKE_CURRENT_SOURCE_DIR}/${way}/${id})
		if (EXISTS ${cn})
			list(APPEND INC_DIR_LIST ${cn})
		endif()
	endforeach()
	#message(STATUS "[${PROJECT_NAME}]  adding_ ${OUR_LIBRARY_DIR}/${OUR_LIBNAME_REPORT}")
endmacro()


# add libs from argn to release and debug lists;   libs are from our "lib" folder
macro(fillOurLibList  releaseLList debugLList)
	set(ourRLib ourRLib-NOTFOUND)
	set(ourDLib ourDLib-NOTFOUND)

	foreach(ourLib ${ARGN})
	    find_library(ourRLib "${ourLib}" PATHS ${OUR_LIBRARY_DIR}/release NO_DEFAULT_PATH)
	    if (NOT ourRLib)
	        message(FATAL_ERROR "error: project ${PROJECT_NAME}: can not find ${ourLib} (release) lib in ${OUR_LIBRARY_DIR}/release")
	    endif()
	    find_library(ourDLib "${ourLib}d" PATHS ${OUR_LIBRARY_DIR}/debug NO_DEFAULT_PATH)
	    if (NOT ourDLib)
	        message(FATAL_ERROR "error: can not find ${ourLib} (debug) lib ")
	    endif()
	    #message(STATUS "....adding " ${ourLib} " libraries " ${ourRLib} " and " ${ourDLib})

	    list(APPEND ${releaseLList}  optimized ${ourRLib})
	    list(APPEND ${debugLList}  debug ${ourDLib})
	    
	    set(ourRLib ourRLib-NOTFOUND)
	    set(ourDLib ourDLib-NOTFOUND)
	endforeach()
endmacro()

#  put some preperties on target tgt
macro (setPropsOnTarget     tgt)
	if (ourLinkOptions) # have some specific optons?
		target_link_options(${tgt} BEFORE PUBLIC ${ourLinkOptions})
	endif()
	
		if (VCPKG_EXISTS)
			message(STATUS "[${tgt}]: using vcpkg toolchain")
			set(TGT_PROPS_WHERE)
		else()
			message(STATUS "[${tgt}]: not using vcpkg toolchain")
			set(TGT_PROPS_WHERE
			ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${OUR_LIBRARY_DIR}/debug"  
			ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${OUR_LIBRARY_DIR}/release"
			ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${OUR_LIBRARY_DIR}/release"
			
			RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUR_LIBRARY_DIR}/release"
			RUNTIME_OUTPUT_DIRECTORY_DEBUG "${OUR_LIBRARY_DIR}/debug"
			RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${OUR_LIBRARY_DIR}/release"
			)
		endif()

	SET_TARGET_PROPERTIES( ${tgt} PROPERTIES
		DEBUG_OUTPUT_NAME ${tgt}d      RELEASE_OUTPUT_NAME ${tgt}    RELWITHDEBINFO_OUTPUT_NAME ${tgt}d  # 'd' suffix rule
		#and put the libs in different folders:
		${TGT_PROPS_WHERE}
		
		#if (VCPKG_EXISTS)
		#	message(STATUS "[${tgt}]: using vcpkg toolchain")
		#else()
		#	message(STATUS "[${tgt}]: not using vcpkg toolchain")
		#	ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${OUR_LIBRARY_DIR}/debug"  
		#	ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${OUR_LIBRARY_DIR}/release"
		#	ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${OUR_LIBRARY_DIR}/release"
		#	
		#	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUR_LIBRARY_DIR}/release"
		#	RUNTIME_OUTPUT_DIRECTORY_DEBUG "${OUR_LIBRARY_DIR}/debug"
		#	RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${OUR_LIBRARY_DIR}/release"
		#endif()
		
		# looks like this is not working:
		VERSION  ${VERSION_INFO_STRING}
		SOVERSION  ${VERSION_INFO_STRING}
		
	)
	if (OUR_DEPS_LIST)
		foreach(lib ${OUR_DEPS_LIST})
			message(STATUS "[${tgt}]: have our lib3: ${lib}")
			add_dependencies(${tgt} ${lib})
		endforeach()
	endif()

	
	if (${now_using_QT5})
		set_target_properties(${tgt} PROPERTIES QT5_NO_LINK_QTMAIN ON)
	endif()
	
	if (${now_using_QT4})
		IF(WIN32)
			if(MSVC) 
				set_target_properties(${tgt} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS") # disable cmd winfdow?
			endif()
		endif()	
		#QT4_AUTOMOC(${PROJ_SRC_FILES})
		set_target_properties(${tgt} PROPERTIES QT4_NO_LINK_QTMAIN ON)
	endif()

	# add version info to the project properties:
	#if(${libType} STREQUAL SHARED) #  DLL special case:
		#SET_TARGET_PROPERTIES( ${tgt} 	PROPERTIES    SOVERSION ${VERSION_INFO_STRING})
		#string(CONCAT CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS} " /VERSION:${VERSION_INFO_STRING}")
	#else()
		#string(CONCAT CMAKE_STATIC_LINKER_FLAGS ${CMAKE_STATIC_LINKER_FLAGS} " /VERSION:${VERSION_INFO_STRING}")
		#SET_TARGET_PROPERTIES( ${tgt}  PROPERTIES     VERSION ${VERSION_INFO_STRING})
	#endif()
endmacro()

macro (commonEnd   libType)
	if(${libType} STREQUAL SHARED) #  DLL special case:
		set(BUILD_SHARED_LIBS ON)
	endif()
	if(${libType} STREQUAL STATIC)
		set(BUILD_SHARED_LIBS OFF)
	endif()
	list(APPEND INC_DIR_LIST ".")
			

	#if (USE_QT)
	#	if (${USE_QT})
			#http://qt-project.org/wiki/toStdWStringAndBuiltInWchar : 
			# not work with boost and sometimes with std set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:wchar_t-")
	#		addQT()
	#	endif()
	#endif()
	
	add_definitions(-DNOMINMAX)
	add_definitions(-DXQBUILDSYSTEM)
	add_definitions(-DOURPROJECTNAME=${PROJECT_NAME}) # this can be used in 'version()' function
	IF(WIN32)
		add_definitions(-DWIN32)
	endif()
	
	add_compile_options("$<$<CONFIG:DEBUG>:-DDEBUG123>") # just for the testing
	
	#message(STATUS "PROJ_SRC_FILES = ${PROJ_SRC_FILES}")
	if (NO_VERSION_INFO)
		message(STATUS "making project without version info (faster compilation)")
	else()
		message(STATUS "making project with version info")
		addVersionInfo(${PROJ_SRC_FILES})
		add_definitions(-DWITH_VERSION_INFO) 
	endif()
	set(src_cur_dir ${CMAKE_CURRENT_SOURCE_DIR})

	if(${libType} STREQUAL EXE)
		set(win32_using)
		if (WIN32 AND now_using_QT)
			if (QT_KEEP_CMD)
				message(STATUS "creating QT project with command prompt")
			else()
				set(win32_using WIN32)
				message(STATUS "creating QT project without command prompt")
			endif()
		else()
			message(STATUS "creating EXE without QT support")
		endif()
		
		if (DEFINED PROJECTS_LIST) #  if we have a list of projects:
			foreach(simpleProjectName ${PROJECTS_LIST})
				# 1. find files related to this project:
				if (EXISTS "${src_cur_dir}/${simpleProjectName}.cc") # gcebatch case
					set(current_src_list "${src_cur_dir}/${simpleProjectName}.cc") # take only one this file
				else()
					# first, look at src folder (geotools batch); after look at special project folder (like in dss utils): 
					if (EXISTS "${src_cur_dir}/src/${simpleProjectName}.cc")
						get_filename_component(srcFileName1 "${src_cur_dir}/src/${simpleProjectName}.cc" ABSOLUTE) # normalize file path
						set(current_src_list ${srcFileName1}) # take only this file
					else() # special progect folder? take everything from it!
						file(GLOB current_src_list ${src_cur_dir}/${simpleProjectName}/*.cc)  # collect all the source files
						
						# add other files, cpp and c:
						file(GLOB clist_1 ${src_cur_dir}/${simpleProjectName}/*.cpp) 
						file(GLOB clist_2 ${src_cur_dir}/${simpleProjectName}/*.c) 
						list(APPEND current_src_list ${clist_1} ${clist_2})
					endif()
				endif()
				
				# 2. process all related files:
				if(current_src_list) # if we have something inside:
					#if (DEFINED SRC_LIST) # this is for version info, etc:
					#	list(APPEND current_src_list ${SRC_LIST})
					#endif()
					
					#addBuildInfo(current_src_list) # bild_info.cc will be created. Where? We do not care. just compile it.
					ADD_EXECUTABLE(${simpleProjectName} ${current_src_list} ${${PROJ_SRC_FILES}}) 
					
					#logMessage("\n\n ============ ADD_EXECUTABLE; simpleProjectName=  ${simpleProjectName}; ============= \n\n current_src_list =  ${current_src_list} \n\n LIB_LIST = ${LIB_LIST} \n\n CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")
		
					#addCommonTargetInfo(${simpleProjectName})
					setPropsOnTarget(${simpleProjectName})
					target_link_libraries(${simpleProjectName}  ${${L_LIST}}   ${${LD_LIST}}   ${${LR_LIST}})  #   ${LIB_LIST}
					#install(TARGETS ${simpleProjectName} RUNTIME DESTINATION bin) # put *.exe to "bin" folder
				else()
					message("WARNING: can not find project item " ${simpleProjectName}";  do we need it really?")
				endif(current_src_list)
			endforeach(simpleProjectName)
		else() # just one single exe file
			ADD_EXECUTABLE(${PROJECT_NAME} ${win32_using} ${${PROJ_SRC_FILES}})
		endif()
		
	else()
		add_library(${PROJECT_NAME}  ${libType} ${${PROJ_SRC_FILES}})
		#message(STATUS "files: ${PROJ_SRC_FILES} = ${${PROJ_SRC_FILES}}")
		set(OUR_LIBNAME_REPORT ${PROJECT_NAME} PARENT_SCOPE)
	endif()
	
	if (NO_VERSION_INFO)
	else()
		addBuildNumber()
	endif()
	if (NEED_BOOST)
		addBoost(${NEED_BOOST})
	endif()
	
	INCLUDE_DIRECTORIES(${INC_DIR_LIST}) 

	message(STATUS "VERSION_INFO_STRING: ${VERSION_INFO_STRING}")
	message(STATUS "CMAKE_SHARED_LINKER_FLAGS: ${CMAKE_SHARED_LINKER_FLAGS}")

	if(WIN32 AND MSVC) 
		add_definitions(/MP  /GS /std:c++14)
		#list(APPEND CMAKE_CXX_FLAGS " /GS /std:c++14 ") # 
		#list(APPEND CMAKE_CXX_FLAGS_RELEASE " /fp:fast /O2  /Ob2 /Oi /Ot /Qpar /openmp /Zc:inline /MD ")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}  /fp:fast /O2  /Ob2 /Oi /Ot /Qpar /openmp /Zc:inline /MD ")
		
		#list(APPEND CMAKE_C_FLAGS_RELWITHDEBINFO  " /Od /Ob0 /Zi /openmp  /MD ")
		set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}  /Od /Ob0 /Zi /openmp  /MD ")
		#list(APPEND CMAKE_CXX_FLAGS_RELWITHDEBINFO " /Od /Ob0 /Zi /openmp  /MD ")
		set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}  /Od /Ob0 /Zi /openmp  /MD ")
		
		#list(APPEND CMAKE_CXX_FLAGS_DEBUG " /MDd ")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd ")
		
		# 4101 unreferenced local variable
		# 4290 C++ exception specification ignored
		# C4018  signed/unsigned mismatch
		#
		
		#set(MSWARNINGS 4267  4996 4290 4101 4018)
		#foreach(wwarning ${MSWARNINGS})
		#	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd${wwarning} ")
		#endforeach()
		
		#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  /wd \\\"4267\\\" /wd\\\"4996\\\"     ")
	endif()
	if (UNIX)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 ")
		add_definitions(-DLIN_UX -DUNIX)
	endif()
	
	#SET_TARGET_PROPERTIES( ${PROJECT_NAME}
	#	PROPERTIES    COMPILE_DEFINITIONS_DEBUG _DEBUG
	#	COMPILE_DEFINITIONS_RELEASE 
	
	#set(CMAKE_CXX_FLAGS_RELEASE  ${CMAKE_CXX_FLAGS_RELEASE} -D_SCL_SECURE_NO_WARNINGS -DNDEBUG -DBUILD123=2 )
	
	foreach(ourGroup  ${ourGroupNames})
		source_group(${ourGroup} FILES ${group_${ourGroup}_files})
		#message(" group ${ourGroup} created; ")
		#message(${group_${ourGroup}_files})
		#message("")
	endforeach()
	
	if (OUR_LIB_LIST) # if we have some libs to add
		fillOurLibList(${LR_LIST}  ${LD_LIST}  ${OUR_LIB_LIST})
	endif()
	if (NOT PROJECTS_LIST)
		setPropsOnTarget(${PROJECT_NAME})
		if((NOT(${libType} STREQUAL STATIC)))
			
			target_link_libraries(${PROJECT_NAME}  ${${L_LIST}}   ${${LD_LIST}}   ${${LR_LIST}} )
			
			#message(STATUS "debug lib list: " )
			#foreach(ourLib123 ${${LD_LIST}})
			#	message(STATUS "      " ${ourLib123})
			#endforeach()
			
			#message("release lib list: " )
			#foreach(ourLib123 ${${LR_LIST}})
			#	message("      " ${ourLib123})
			#endforeach()
			#message(" _common_ lib list: " )
			#foreach(ourLib123 ${${L_LIST}})
			#	message("      " ${ourLib123})
			#endforeach()


		endif()
	endif()

	
	if (UNIX)
		if(${libType} STREQUAL SHARED) #  DLL special case:
			if( CMAKE_SIZEOF_VOID_P MATCHES 8 )
				install(TARGETS ${PROJECT_NAME}   DESTINATION lib64)
			else()
				install(TARGETS ${PROJECT_NAME}   DESTINATION lib)
			endif()
		endif()
	endif()
	

endmacro()

# just copy our target somewhere as a post build step
macro (copy_pb whereToCopy)
	if (DEFINED PROJECTS_LIST) #  if we have a list of projects:
		# do nothing?
	else()
		message(STATUS "[${PROJECT_NAME}] copy_pb: putting target to ${whereToCopy}" )
		add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${whereToCopy}/$<TARGET_FILE_NAME:${PROJECT_NAME}>)
		
	endif()
endmacro()

macro (programEnd)
	commonEnd(EXE)
endmacro()

macro (libraryEnd libType)
	commonEnd(${libType})
endmacro()



