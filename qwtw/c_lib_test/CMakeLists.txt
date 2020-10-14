#
# 	# qwtwc library test project
#

cmake_minimum_required (VERSION 2.8.11)
include("${CMAKE_CURRENT_SOURCE_DIR}/../../buildsys/ccmake.cmake")
set(USE_QT YES)
set (NO_VERSION_INFO 1)

set(PROJECT_NAME qwtw_test)
set (NEED_BOOST  thread system filesystem date_time chrono)
commonStart()
set(INC_DIR_LIST "../" "../c_lib"  "../../../common" "../../../xmatrix2" )
	
addSourceFiles("source" "../" qwtwtest)	

add_definitions(-DUNICODE -D_UNICODE) # -DUSE_QT3D )
#add_definitions(-DUSEMARBLE)

#addOurLib3(../c_lib)
list(APPEND ${L_LIST} qwtw)
list(APPEND INC_DIR_LIST "../c_lib")

programEnd()
install(TARGETS ${PROJECT_NAME}  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/tools/qwtw)

# ========================================================


