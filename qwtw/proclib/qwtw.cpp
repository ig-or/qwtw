

#include "xstdef.h"
#include "qwtw.h"
#include "xmutils.h"
#include "build_info.h"
#include "build_number.h"

#include "pclient.h"
#include "qwtypes.h"
#include <chrono>
#include <thread>

#ifdef WIN32
#include <windows.h>
HMODULE qwtwLibModule = 0;
#endif
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */


void assert_failed(const char* file, unsigned int line, const char* str) {
	xmprintf(0, "ASSERT faild: %s (file %s, line %d)\n", str, file, line);
}
int xmPrintLevel = 0; // will printf messages with level <= then this
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
 **/ 

#ifdef USEMARBLE
qwtwc_API	int qtstart(const char* mdp, const char* mpp) {
	std::string mdPath;
	std::string mpPath;
	if (mdp == 0) {
		mdPath.clear();
	} else {
		mdPath.assign(mdp);
	}
	if (mpp == 0) {
		mpPath.clear();
	} else {
		mpPath.assign(mpp);
	}

	return test.testInit(mdPath, mpPath);  //  default level == 1 ?
}

qwtwc_API void setcallback(OnPCallback pCallback) {
	test.setCB(pCallback);
}

qwtwc_API   void setclipcallback(OnClipCallback pCallback) {
	test.setClipCallback(pCallback);
}

qwtwc_API   void setcallback_t1(CBTest_1 pCallback) {
	test.setCBTest1(pCallback);
}
qwtwc_API   void setcallback_t2(CBTest_2 pCallback) {
	test.setCBTest2(pCallback);
}

qwtwc_API	int qtstart_debug(const char* mdp, const char* mpp, int level) {
	std::string mdPath;
	std::string mpPath;
	if (mdp == 0) {
		mdPath.clear();
	} else {
		mdPath.assign(mdp);
	}
	if (mpp == 0) {
		mpPath.clear();
	} else {
		mpPath.assign(mpp);
	}
	xmPrintLevel = level;
	return test.testInit(mdPath, mpPath, level);
}
#else
qwtwc_API	int qtstart() {
	return test.testInit();  //    default level == 1 ?
}

qwtwc_API	int qtstart_debug(int level) {
	xmPrintLevel = level;
	return test.testInit(level);
}
#endif
qwtwc_API	void qwtclose() {
	test.stopQt();
}


/**  print version info string (with 'sprintf'). warning: no vstr length check!!
	@param[out] vstr string with version info
	@return number of bytes in vstr

*/
qwtwc_API		int qwtversion(char* vstr, int vstr_size) {
#ifdef WIN32
	if (qwtwLibModule == 0) {
		return xqversion(vstr, vstr_size - 1);
	}	else {
		return xqversion(vstr, vstr_size - 1, qwtwLibModule);
	}
#else	//  linux ? 
	return xqversion(vstr, vstr_size - 1);
#endif
}


/**  create (and draw) new plot with ID 'n';  make this plot 'active'
     if plot with this ID already exists, it will be made 'active'
 @param[in] n this plot ID
*/
qwtwc_API 	int qwtfigure(int n, unsigned int flags) {
	return test.qwtfigure(n, flags);
}

qwtwc_API 	void qwtclipgroup(int gr) {
	test.qwtClipGroup(gr);
}

#ifdef USEMARBLE
/**  create (and draw) new map plot with ID 'n'.
@param[in] n this plot ID
*/
qwtwc_API 	int qwtmap(int n) {
	return test.qwtmap(n);
}
#endif
#ifdef USE_QT3D
qwtwc_API 	void qwtfigure3d(int n);
#endif

qwtwc_API 	int qwtservice(int x) {
	return test.qwtservice(x);
}

#ifdef USEMATHGL

qwtwc_API 	int qwtmgl(int n){
	return test.qwtmgl(n);
}

qwtwc_API 	void qwtmgl_line(int size, double* x, double* y, double* z, const char* name, const char* style) {
	test.qwtmgl_line(size, x, y, z, name, style);
}

qwtwc_API 	void qwtmgl_mesh(int xSize, int ySize, 
		double xMin, double xMax, double yMin, double yMax, 
		double* data, const char* name,
		const char* style,
		int type) {
			SurfDataType sd = static_cast<SurfDataType>(type);
			test.qwtmgl_mesh(MeshInfo{xSize, ySize, xMin, xMax, yMin, yMax, data, name, style, sd});
		}
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

qwtwc_API 	int qwtplot(double* x, double* y, int size, const char* name, const char* style, 
	    int lineWidth, int symSize) {
	return test.qwtplot(x, y, size, name, style, lineWidth, symSize);
}

qwtwc_API 	int qwtplot2(double* x, double* y, int size, const char* name, const char* style, 
	    int lineWidth, int symSize, double* time) {
	return test.qwtplot2(x, y, size, name, style, lineWidth, symSize, time);
}

qwtwc_API void qwtremove(int id) {
	test.qwtremove(id);
}

qwtwc_API int qwtchange(int id, double* x, double* y, double* z, double* time, int size) {
	return test.qwtchange(id, x, y, z, time, size);
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
	test.qwtshowmw();
}


#ifdef WIN32
BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
) {
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		qwtwLibModule = hModule;
		//++prCounter;
		//printf("DLL main: DLL_PROCESS_ATTACH; prCounter = %d\n", prCounter);
		break;
	case DLL_THREAD_ATTACH:
		//++trCounter;
		//printf("DLL main: DLL_THREAD_ATTACH; trCounter = %d\n", trCounter);
		break;
	case DLL_THREAD_DETACH:
		//--trCounter;
		//printf("DLL main: DLL_THREAD_DETACH; trCounter = %d\n", trCounter);
		break;
	case DLL_PROCESS_DETACH:
		//--prCounter;
		//printf("DLL main: DLL_PROCESS_DETACH; prCounter = %d\n", prCounter);
		break;
		break;
	}
	return TRUE;
}

#endif


#ifdef __cplusplus
	}
#endif


static const int logBufLen = 2048;
static char logBuf[logBufLen];
int xmprintf(int level, const char * _Format, ...) {
	using namespace std::chrono_literals;
	if (level > xmPrintLevel) {
		return 1;
	}
	va_list args;
	va_start(args, _Format);

	int ok = vsnprintf(logBuf, logBufLen, _Format, args);
	logBuf[logBufLen - 1] = 0;
	if(ok > 0) { // we got the message
		printf("%d-%d \t%s", xmPrintLevel, level, logBuf);
		//std::cout << logBuf;
	}
	va_end(args);
	if (xmPrintLevel > 5) {
		std::this_thread::sleep_for(10ms);
	}
	return 0;
}


