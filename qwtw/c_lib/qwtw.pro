#
#	qwtw PRO file
#

#$(LocalDebuggerEnvironment)
#
# qwtPath env var should be defined


QT *= opengl
CONFIG += dll use_boost
DEFINES += QT_DLL qwtwEXPORTS qqwtwEXPORTS QWT_DLL 
projectname = qwtw
extlibs = ../extlib
include(../common/qtpro.pri)

win32{
}
unix {
	QMAKE_CXXFLAGS=-std=c++11
	LIBS += $$(qwtPath)/lib/libqwt.so
}

INCLUDEPATH += ../xmatrix2 ../common

INCLUDEPATH += $$(qwtPath)/include
LIBPATH += $$(qwtPath)/lib

CONFIG(release, release|debug) { # ===================   Release  ============================
    win32{
		LIBS += qwt.lib
	}
    unix{
       # LIBS += $$(qwtPath)/libqwt.so
    }
	#DEF_FILE = gridselectordef.def
} else {   # ===================   DEBUG CONFIG  ====================================
    win32{
		LIBS += qwtd.lib
    }
    unix {  
     #   LIBS += libqwt.so
    }
	#DEF_FILE = gridselectordef_d.def
}


##################################################

matrix = 	xmatrix2 	xmatrixplatform xmatrix_imp xmcoords  xmroundbuf 
commonFiles = xstdef xqbytebuffer xmutils qtswidget  blockingqueue thread 
qwtwFiles = sfigure figure2 justaplot	qwtw qtint	
klifft = complex fft psd
		
SOURCES += $$addSources("../common"	, $${commonFiles})
SOURCES += $$addSources("../common/klifft"	, $${klifft})
SOURCES += $$addSources("../xmatrix2"	, $${matrix})				
SOURCES += $$addSources("./"	, $${qwtwFiles})	

#SOURCES += $$extlibs/fftw/src/fftw++.cc

HEADERS += $$addHeaders("../common"	, $${commonFiles})
HEADERS += $$addHeaders("../common/klifft"	, $${klifft})
HEADERS += $$addHeaders("../xmatrix2"	, $${matrix})				
HEADERS += $$addHeaders("./"	, $${qwtwFiles})	
			
FORMS += lineselect.ui 
	 
RESOURCES     = plib1.qrc









