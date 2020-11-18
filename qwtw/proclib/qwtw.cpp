

#include "xstdef.h"
#include "qwtw.h"
#include "xmutils.h"
#include "build_info.h"
#include "build_number.h"

#include "pclient.h"


#ifdef WIN32
#include <Windows.h>
extern HMODULE qwtwLibModule;
#endif


void assert_failed(const char* file, unsigned int line, const char* str) {
	xm_printf("ASSERT faild: %s (file %s, line %d)\n", str, file, line);
}

SHMTest test;

#ifdef __cplusplus
	extern "C" {
#endif

/**  this is just for testing*/
qwtwc_API 	int get42(int n) {
	return 42;
}

qwtwc_API void kyleHello() {

}

/** starting  QT proc, if not started yet.
 * 	return 0 if all is OK, 
 *  not 0 means nothing is working.
 * */ 

qwtwc_API	int qtstart() {
	return test.testInit();
}

qwtwc_API	void qwtclose() {
	test.stopQt();
}


/**  print version info string (with 'sprintf'). warning: no vstr length check!!
	@param[out] vstr string with version info
	@return number of bytes in vstr

*/
qwtwc_API		int qwtversion(char* vstr, int vstr_size) {
	return xqversion(vstr, vstr_size - 1);
}


/**  create (and draw) new plot with ID 'n';  make this plot 'active'
     if plot with this ID already exists, it will be made 'active'
 @param[in] n this plot ID
*/
qwtwc_API 	void qwtfigure(int n) {
	test.qwtfigure(n);
}

#ifdef USEMARBLE
/**  create (and draw) new map plot with ID 'n'.
@param[in] n this plot ID
*/
qwtwc_API 	void topview(int n);
#endif
#ifdef USE_QT3D
qwtwc_API 	void qwtfigure3d(int n);
#endif

/** put a title on currently 'active' plot.
	@param[in] s the title
*/
qwtwc_API 	void qwttitle(const char* s) {
	test.qwttitle(s);
}

/** put a 'label' on X (bottom) axis.
	@param[s] axis name string
*/
qwtwc_API 	void qwtxlabel(const char* s) {
	test.qwtxlabel(s);
}

/** put a 'label' on Y (left) axis.
@param[s] axis name string
*/
qwtwc_API 	void qwtylabel(const char* s) {
	test.qwtylabel(s);
}

/** close all figures.
*/
qwtwc_API 	void qwtclear() {
	test.qwtclear();
}

/** This function sets some additional flags on (following) lines.
	@param[in] status if '0', all next lines will be "not important":
	 this means that they will not participate in 'clipping' and after pressing "clip" button
	 thier range will not be considered.
	 '1' would return this backward.
*/
qwtwc_API 	void qwtsetimpstatus(int status) {
	test.qwtsetimpstatus(status);
}

qwtwc_API 	void qwtplot(double* x, double* y, int size, const char* name, const char* style, 
	    int lineWidth, int symSize) {
	test.qwtplot(x, y, size, name, style, lineWidth, symSize);
}

qwtwc_API 	void qwtplot2(double* x, double* y, int size, const char* name, const char* style, 
	    int lineWidth, int symSize, double* time) {
	test.qwtplot2(x, y, size, name, style, lineWidth, symSize, time);
	}
#ifdef USE_QT3D
qwtwc_API 	void qwtplot3d(double* x, double* y, double* z, int size, const char* name, const char* style,
	int lineWidth, int symSize, double* time);
#endif

#ifdef ENABLE_UDP_SYNC
qwtwc_API 	void qwtEnableCoordBroadcast(double* x, double* y, double* z, double* time, int size) {
	test.qwtEnableCoordBroadcast(x, y, z, time, size);
}
qwtwc_API 	void qwtDisableCoordBroadcast() {
	test.qwtDisableCoordBroadcast();
}
#endif

/** do not use it if all is working without it.
    This function will try to "close" QT library.  Craches sometimes.
*/
//qwtwc_API 	void qwtclose(); //  works strange

/** Show 'main window' which allow to easily switch between other windows.

*/
qwtwc_API 	void qwtshowmw() {

}





#ifdef __cplusplus
	}
#endif

