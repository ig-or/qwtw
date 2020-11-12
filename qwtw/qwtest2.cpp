/** very simple qwtw library test.
	This test will try to dynamically load the library.
*/

#include "stdio.h"
#include "stdlib.h"
#include <iostream>
#include <math.h>
 
#ifdef WIN32
	#include <conio.h>
	#include <Shlobj.h>
	#include <tchar.h>
#else
	#include <sys/types.h>
	#include <dlfcn.h>
#endif

#include <iomanip>
#include <locale>
#include <codecvt>
#include <sstream>
#include <string>
#include <chrono>
#include <thread>

#include "boost/filesystem.hpp"

//  ==========  path #1. lets create some data ===================================
//   our data arrays below:
const int n1 = 24;
const double maxIks = 10.;
double sinusTime_1[n1], sinusData_1[n1];

const int nc = 16, n3 = 40, n4 = 4000;
//  data for circle line:
double circleData_x1[nc], circleData_y1[nc], circleTime_1[nc], circleRadius_1 = 1.4; //  x, y, time info, radius

double* x_sinusData_2;
double* y_sinusData_2;
double* x4;
double* y4;
double* y5;
double* y6;

const int n3d = 100;
double* x3d;
const int n3d2 = 5;
double* x3d2;

const int nns = 10000;
double xs[nns], ys[nns], ys1[nns];
double delta;

const int mwn = 4;
double north[mwn] = {55.688713, 55.698713, 55.678713, 55.60};
double east[mwn] = {37.901073, 37.911073, 37.905073, 37.9};
double t4[mwn];

///  this function will fill in data arrays
void createInfo();

///   this function will try to call the library
void test(int n);

int main(int argc, char* argv[]) {
	createInfo(); //    fill in all data arrays

	setbuf(stdout, NULL);
	setvbuf(stdout, NULL, _IONBF, 0); 

	for (int i = 0; i < 1; i++) {
		test(i);
	}
	 
	//  clean memory
	delete[] x_sinusData_2;
	delete[] y_sinusData_2;
	delete[] x4;
	delete[] y4;
	delete[] y5;
	delete[] y6;
	delete[] x3d;
	return 0;
}

//  the library instance
//static volatile  HINSTANCE hQWTW_DLL = 0;

void test(int n) {
	printf("\n\n\ndoing the test # %d\n", n);
	using namespace std::chrono_literals;
	//std::this_thread::sleep_for(100ms);

	//  where our EXE is located? lets find out
	std::string exeFileName;
	#ifdef WIN32
	TCHAR buf[MAX_PATH];
	int ok = GetModuleFileName(0, buf, MAX_PATH);
	if (ok == 0) { // and what?
		std::cout << "TRACE:  GetModuleFileName failed  " << std::endl;
		return;
	} 
	exeFileName = buf;
	#else
		char arg1[32];
		char exepath[2014] = {0};
		sprintf( arg1, "/proc/%d/exe", getpid() );
		int test = readlink( arg1, exepath, 1020 );
		if (test > 0) {
			exeFileName.assign(exepath);
		} else {
			exeFileName = "/home";
		}
	#endif
	std::cout << "exeFileName = " << exeFileName << std::endl;

	using namespace boost::filesystem;

	//  at first, lets load the library form the same folder
	path p = ((path)(exeFileName)).parent_path();

	#ifdef WIN32
#ifdef DEBUG123
	std::string qwtw_name = "qwtwd.dll";
#else
	std::string qwtw_name = "qwtw.dll";
#endif
	std::string rdfDllName = (p / qwtw_name).string();

	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring wide = converter.from_bytes(p.string());

	AddDllDirectory(wide.c_str());
	std::cout << "loading the library " << rdfDllName << std::endl;

	hQWTW_DLL = LoadLibraryEx(rdfDllName.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
	if (!hQWTW_DLL) {
		DWORD dw = GetLastError();

		std::cout << "TRACE: cannot load library " << rdfDllName << " ; error code = " << dw << ";  trying default location ... " <<std::endl;

		//  now, lets try simplle way
		hQWTW_DLL = LoadLibrary(qwtw_name.c_str());
		if (!hQWTW_DLL) {
			DWORD dw = GetLastError();
			std::cout << "TRACE: cannot load library " << qwtw_name << " ; error code = " << dw  << std::endl;
			return;
		}
		else {
			std::cout << "library loaded! " << std::endl;
		}
	}	else {
		std::cout << "library loaded! " << std::endl;
	}
	#else
 		void *lib_handle;
		#ifdef DEBUG123
		std::string qwtw_name = "libqwtwd.so";
		#else
		std::string qwtw_name = "libqwtw.so";
		#endif
		std::string qwtwPath = (p / qwtw_name).string();
		lib_handle = dlopen(qwtwPath.c_str(), RTLD_LAZY);
		if (!lib_handle)	   {
			printf("can not load library from %s: %s;   trying another way..\n", qwtwPath.c_str(), dlerror());
			lib_handle = dlopen(qwtw_name.c_str(), RTLD_LAZY);
			if (!lib_handle)	   {
				printf("can not load library %s: %s\n", qwtw_name.c_str(), dlerror());
				exit(1);
			} else {
				printf("%s loaded \n", qwtw_name.c_str());
			}
	   } else {
		   printf("%s loaded \n", qwtwPath.c_str());
	   }
	#endif

	//  some of the function prototypes are below:
	typedef void(*pQSimple)();
	typedef void(*pQSimple1)(int);
	typedef void(*pQSimple2)(const char*);
	typedef int(*pVersion)(char*, int);
	typedef int(*pQ2Start)();
	typedef void (*pPlot)(double*, double*, int, const char*, const char*, int, int);
	typedef void (*pPlot2)(double*, double*, int, const char*, const char*, int, int, double*);

#ifdef WIN32
	pQSimple qHello = (pQSimple)GetProcAddress(hQWTW_DLL, "kyleHello");
	pQSimple qClose = (pQSimple)GetProcAddress(hQWTW_DLL, "qwtclose");
	pQSimple2 qXLabel = (pQSimple2)GetProcAddress(hQWTW_DLL, "qwtxlabel");
	pQSimple2 qYLabel = (pQSimple2)GetProcAddress(hQWTW_DLL, "qwtylabel");
	pVersion qVersion = (pVersion)GetProcAddress(hQWTW_DLL, "qwtversion");
	pQ2Start q2Start = (pQ2Start)GetProcAddress(hQWTW_DLL, "qtstart");
	pQSimple qSMW = (pQSimple)GetProcAddress(hQWTW_DLL, "qwtshowmw");
	pQSimple1 qTopView = (pQSimple1)GetProcAddress(hQWTW_DLL, "topview");
	pQSimple1 qFigure = (pQSimple1)GetProcAddress(hQWTW_DLL, "qwtfigure");
	pQSimple2 qTitle = (pQSimple2)GetProcAddress(hQWTW_DLL, "qwttitle");
	pPlot2 qPlot2 = (pPlot2)GetProcAddress(hQWTW_DLL, "qwtplot2");
	pPlot qPlot = (pPlot)GetProcAddress(hQWTW_DLL, "qwtplot");
	//void qwtplot(double* x, double* y, int size, const char* name, const char* style, int lineWidth, int symSize);
#else
	pQSimple qHello = (pQSimple)dlsym(lib_handle, "kyleHello");
	pQSimple qClose = (pQSimple)dlsym(lib_handle, "qwtclose");
	pQSimple2 qXLabel = (pQSimple2)dlsym(lib_handle, "qwtxlabel");
	pQSimple2 qYLabel = (pQSimple2)dlsym(lib_handle, "qwtylabel");
	pVersion qVersion = (pVersion)dlsym(lib_handle, "qwtversion");
	pQ2Start q2Start = (pQ2Start)dlsym(lib_handle, "qtstart");
	pQSimple qSMW = (pQSimple)dlsym(lib_handle, "qwtshowmw");
	#ifdef USEMARBLE
	pQSimple1 qTopView = (pQSimple1)dlsym(lib_handle, "topview");
	#endif
	pQSimple1 qFigure = (pQSimple1)dlsym(lib_handle, "qwtfigure");
	pQSimple2 qTitle = (pQSimple2)dlsym(lib_handle, "qwttitle");
	pPlot2 qPlot2 = (pPlot2)dlsym(lib_handle, "qwtplot2");
	pPlot qPlot = (pPlot)dlsym(lib_handle, "qwtplot");
#endif

	if ((qHello == NULL) || (q2Start == NULL) || (qVersion == NULL)) {
		printf("ERROR: cannot load symbols. \n");
		exit(2);
	}
	//printf("exiting from TEST\n");
	//return;

	
	//std::this_thread::sleep_for(100ms);
	int test0 = q2Start();
	std::cout << "test 0 = " << test0 << std::endl;

	char vi[256];
	int test1 = qVersion(vi, 256);
	std::cout << " version: " << vi << std::endl;

	if (qHello == 0) {
		std::cout << "qHello = 0 " << std::endl;
	}else {
		for (int i = 0; i < 1; i++) {
			qHello();
		}
	} 

	//  open """main""" window
	qSMW();

	// --------   map view test: draw on a map  ---------------------
#ifdef USEMARBLE
	t4[0] = sinusTime_1[0]; 
	t4[1] = sinusTime_1[int(n1 / 4.)];
	t4[2] = sinusTime_1[int(n1 / 3.)];
	t4[3] = sinusTime_1[int(n1 / 2.)];
	qTopView(1);
	qPlot2(east, north, mwn, "test mw", "-rb", 2, 8, t4);
	qTitle("marble test");
#endif
	
	// ------------- create first plot: ---------------------
	qFigure(14);	//  set up ID for current plot

	qXLabel("x axis label"); //  put X axis label
	qYLabel("Y axis label"); // put Y label
	qTitle("title for first plot"); // set up a title

	// draw sinus line with "t-b" (blue line) style and line width =2
	qPlot(sinusTime_1, sinusData_1, n1, "sinusTime_1 plot", "-tb", 2, 24);
	//  add a circle "-m" (magenta color) to the same plot
	qPlot(circleData_x1, circleData_y1, nc, "circle", "-qm", 2, 16);
	//  add one more sinus here, darkMagenta color
	qPlot(x4, y4, n4, "x_sinusData_2 plot", "-M", 1, 1);

	// --------------- one more plot --------------------------------
	qFigure(7);
	qPlot(x_sinusData_2, y_sinusData_2, n3, "one more plot", "-r", 1, 1); //  thin line, red color
	qPlot(x4, y4, n4, "x4 plot", "-r", 2, 1);
	qPlot(x4, y5, n4, "x5 plot", "-k", 2, 1);
	qTitle("qwtw  testing");     qXLabel("[seconds ?]");     qYLabel("sinuses %)");

	// ----- sinus and circle together on this plot ----------
	qFigure(3);
	qPlot(x4, y6, n4, "x6 plot", "-b", 22, 1);
	//  add a circle "-m" (magenta color) to the same plot
	qPlot(circleData_x1, circleData_y1, nc, "circle", "-qm", 2, 16);
	qXLabel("[seconds ?]"); 	qYLabel("y axis!"); 	qTitle("sinus and circle");

	// play with a marker here on this plot
	qFigure(10);
	qPlot2(circleData_x1, circleData_y1, nc, "circle", "-qm", 1, 12, circleTime_1);
	qTitle("'top view' test");

	//std::this_thread::sleep_for(5s);
	//qClose();
	//return;

	std::cout << "press a key to close the DLL:" << std::endl;
	int  ch;
	#ifdef WIN32
	ch = getch();
	#else
	std::cin >> ch;
	#endif
	//

	std::this_thread::sleep_for(400ms);
	std::cout << "closing the library " << std::endl;
	std::this_thread::sleep_for(400ms);

	#ifdef WIN32
	BOOL test = FreeLibrary(hQWTW_DLL);
	if (test == 0) {
		std::cout << "FreeLibrary failed; error = " << GetLastError() << std::endl;
	}
	hQWTW_DLL = 0;
	#else
	dlclose(lib_handle);
	lib_handle = 0;
	#endif
	std::cout << "library unloaded! " << std::endl;
}

void createInfo() {
	int i;
	x_sinusData_2 = new double[n3];
	y_sinusData_2 = new double[n3];
	x4 = new double[n4];
	y4 = new double[n4];
	y5 = new double[n4];
	y6 = new double[n4];

	//delta = x_sinusData_2[n3 - 1] - x_sinusData_2[0];

	//  1.  create some data: simple sine line
	for (i = 0; i < n1; i++) {
		sinusTime_1[i] = i * maxIks / n1;
		sinusData_1[i] = sin(sinusTime_1[i]);
	}

	for (i = 0; i < nc; i++) {
		double  a = (double)(i)*360. / (double)(nc);
		circleTime_1[i] = a * 3.14159 / 180.;

		circleData_x1[i] = sin(circleTime_1[i])* circleRadius_1;
		circleData_y1[i] = cos(circleTime_1[i]) * circleRadius_1;
	}

	// another sinus:
	for (i = 0; i < n3; i++) {
		x_sinusData_2[i] = (i*maxIks) / n3;
		y_sinusData_2[i] = cos(x_sinusData_2[i] * 0.9) * sin(x_sinusData_2[i] * 1.1);
	}


	for (i = 0; i < n4; i++) {
		x4[i] = (i*maxIks) / n4;
		y4[i] = cos(x4[i] * 0.9) + sin(x4[i] * 1.1);
		y5[i] = cos(x4[i] * 0.4) - sin(x4[i] * 1.5);
		y6[i] = cos(x4[i] * 0.2) + sin(x4[i] * 1.15);
	}


	x3d = new double[n3d * 4];
	double r = 5.;
	for (i = 0; i < n3d; i++) {
		x3d[i] = i * 0.1; //  time?
		double a = i * 2. * 3.14159 * 4. / (double)(n3d);
		x3d[i + n3d] = sin(a) * r;
		x3d[i + 2*n3d] = cos(a) * r;
		x3d[i + 3 * n3d] = i * 8. / ((double)(n3d));
	}


	x3d2 = new double[n3d2 * 4];
	r = 8.;
	for (i = 0; i < n3d2; i++) {
		x3d2[i] = i * 0.1 * n3d/n3d2; //  time?
		double a = i * 2. * 3.14159 * 0.8 / (double)(n3d2);
		x3d2[i + n3d2] = sin(a) * r;
		x3d2[i + 2 * n3d2] = cos(a) * r;
		x3d2[i + 3 * n3d2] = 0.;
	}
}




