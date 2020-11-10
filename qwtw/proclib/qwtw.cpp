

#include "xstdef.h"
#include "qwtw.h"

#include "qtint.h"
#include "xmutils.h"
#include "build_info.h"
#include "build_number.h"

#ifdef WIN32
#include <Windows.h>
extern HMODULE qwtwLibModule;
#endif


void assert_failed(const char* file, unsigned int line, const char* str) {
	xm_printf("ASSERT faild: %s (file %s, line %d)\n", str, file, line);
}

//void startQWT();

#ifdef __cplusplus
	extern "C" {
#endif

qwtwc_API int get42(int n) {
    return 42;
}

/*
qwtwc_API int qwtversion(char* vstr, int vstr_size) {
#ifdef WIN32
	return xqversion(vstr, vstr_size - 1, qwtwLibModule);
#else
	strcpy(vstr, " linux? ");
	
#endif
	return 0;
}
*/

/*
qwtwc_API void qwtfigure(int n) {
	if (qwtController == 0) {
		startQWT();
	}
	if (qwtController != 0 ) {
		qwtController->figure(n);
	}
}
*/
/*
#ifdef USEMARBLE
qwtwc_API void topview(int n) {
	if (qwtController == 0) {
		startQWT();
	}
	if (qwtController != 0) {
		qwtController->figure_topview(n);
	}
}
#endif
*/
#ifdef USE_QT3D
/*
qwtwc_API 	void qwtfigure3d(int n) {
	if (qwtController == 0) {
		startQWT();
	}
	if (qwtController != 0) {
		qwtController->figure_3d(n);
	}
}
*/
#endif
/*
qwtwc_API void qwttitle(char* s) {
	if (qwtController != 0 )
	qwtController->title(s);
}
*/
/*
void qwtaxesequal() {
	if (qwtController != 0 )
	qwtController->axesequal();
}
*/
/*
qwtwc_API void qwtxlabel(char* s) {
	if (qwtController != 0 )
	qwtController->xlabel(s);
}


qwtwc_API void qwtylabel(char* s) {
	if (qwtController != 0 )
	qwtController->ylabel(s);
}
*/
/*
void qwtmode(int mode) {
    if(qwtController != 0)
	   qwtController->setmode(mode);
}
*/
/*
qwtwc_API 	void qwtclear() {
	if (qwtController != 0) {
		qwtController->clear();
	}
}
*/
/*
qwtwc_API 	void qwtsetimpstatus(int status) {
	if (qwtController != 0) {
		qwtController->setImportantStatus(status);
	}
}
*/
/*
qwtwc_API void qwtplot(double* x, double* y, int size, char* name, const char* style, int lineWidth, int symSize) {
    if(qwtController != 0) {
	   qwtController->plot(x, y, size, name, style, lineWidth, symSize);
    }
}
*/
/*
qwtwc_API 	void qwtshowmw() {
	if (qwtController != 0) {
		qwtController->showMW();
	}
}
*/

/*
void qwtplot2(double* x, double* y, int size, char* name, const char* style,
    int lineWidth, int symSize, double* time) {
    if(qwtController != 0) {
	   qwtController->setmode(3);
	   qwtController->plot(x, y, size, name, style, lineWidth, symSize, time);
    }
}
*/

/*
#ifdef ENABLE_UDP_SYNC
qwtwc_API 	void qwtEnableCoordBroadcast(double* x, double* y, double* z, double* time, int size) {
	if (qwtController != 0) {
		qwtController->qwtEnableCoordBroadcast(x, y, z, time, size);
	}
}

qwtwc_API 	void qwtDisableCoordBroadcast() {
	if (qwtController != 0) {
		qwtController->qwtDisableCoordBroadcast();
	}
}
#endif
*/
#ifdef USE_QT3D
/*
void qwtplot3d(double* x, double* y, double* z, int size, char* name, const char* style,
	int lineWidth, int symSize, double* time) {
	if (qwtController != 0) {
		//qwtController->setmode(3);
		qwtController->plot(x, y, z, size, name, style, lineWidth, symSize, time);
	}
}
*/
#endif

 //  not working!
/*
qwtwc_API void qwtclose() {
	xm_printf("got qwtwc_API void qwtclose() \n");
	if (qwtController != 0 ) {
		qwtController->close();

		delete qwtController;
		qwtController = 0;
	}
}
*/
#ifdef __cplusplus
	}
#endif

