#
# 	# marble library test project
#

cmake_minimum_required(VERSION 3.21)
include("${CMAKE_CURRENT_SOURCE_DIR}/../../buildsys/ccmake.cmake")
set(USE_QT YES)

set(PROJECT_NAME mtest)
set (NEED_BOOST thread system filesystem date_time chrono)

commonStart()

set(INC_DIR_LIST "../"   "../../common" "../../xmatrix2" )

	
addSourceFiles("source" "."  mtest)	
#addSourceFiles("qwtw" "../" sfigure figure2 justaplot )	
#addSourceFiles("matrix" "../../xmatrix2" xmatrix2 	xmatrixplatform xmatrix_imp xmcoords )

addSourceFiles("common" "../../common"
	xstdef xmutils
	#xqbytebuffer  queue_block qtswidget xmroundbuf
	#sqwlinetree lrbuf
)

#addSourceFiles("klifft" "../../common/klifft"	complex fft psd )

#list(APPEND SRC_LIST  ../plib1.qrc)

add_definitions(-DUNICODE -D_UNICODE)
#add_definitions(-DQT_DLL -DQWT_DLL -DqqwtwEXPORTS -DqwtwcEXPORTS)

if(WIN32)
	addExtLibrary(marble VSTRING 1.11.3 RLNAME marblewidget-qt5  )
else()
	if(UNIX AND NOT APPLE)
		add_definitions(-DLIN_UX)
	endif()
	addExtLibrary(marble VSTRING 1.11.3 RLNAME marblewidget-qt5  )
endif()
programEnd()


