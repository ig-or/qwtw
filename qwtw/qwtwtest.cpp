/** very simple qwtw library demo.

*/

#include "qwtw.h" // do not forget the header file

#include "stdio.h"
#include "stdlib.h"
#include <iostream>
#include <math.h>
#include <conio.h>

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

//  this function will fill in data arrays
void createInfo();

int main(int argc, char* argv[]) {
	createInfo(); //    fill in all data arrays
	qtstart();

	//  3D test:
#ifdef USE_QT3D
	qwtfigure3d(0);

	qwtplot3d(x3d + n3d, x3d + 2 * n3d, x3d + 3 * n3d, n3d, "3D test #2", "-r", 2, 5, x3d);
	qwtplot3d(x3d2 + n3d2, x3d2 + 2 * n3d2, x3d2 + 3 * n3d2, n3d2, "3D test #3", "-g", 2, 5, x3d2);
#endif


	//goto end;
	// ------------- create first plot: ---------------------
	qwtfigure(14);	//  set up ID for current plot

	qwtxlabel("x axis label"); //  put X axis label
	qwtylabel("Y axis label"); // put Y label
	qwttitle("title for first plot"); // set up a title

	// draw sinus line with "t-b" (blue line) style and line width =2
	qwtplot(sinusTime_1, sinusData_1, n1, "sinusTime_1 plot", "-tb", 2, 24);
	//  add a circle "-m" (magenta color) to the same plot
	qwtplot(circleData_x1, circleData_y1, nc, "circle", "-qm", 2, 16);
	//  add one more sinus here, darkMagenta color
	qwtplot(x4, y4, n4, "x_sinusData_2 plot", "-M", 1, 1);

	// --------------- one more plot --------------------------------
	qwtfigure(7);
	qwtplot(x_sinusData_2, y_sinusData_2, n3, "one more plot", "-r", 1, 1); //  thin line, red color
	qwtplot(x4, y4, n4, "x4 plot", "-r", 2, 1);
	qwtplot(x4, y5, n4, "x5 plot", "-k", 2, 1);
	qwttitle("qwtw  testing");     qwtxlabel("[seconds ?]");     qwtylabel("sinuses %)");
    
	// ----- sinus and circle together on this plot ----------
	qwtfigure(3);
	qwtplot(x4, y6, n4, "x6 plot", "-b", 22, 1);
	//  add a circle "-m" (magenta color) to the same plot
	qwtplot(circleData_x1, circleData_y1, nc, "circle", "-qm", 2, 16);
	qwtxlabel("[seconds ?]"); 	qwtylabel("y axis!"); 	qwttitle("sinus and circle");
	
	// play with a marker here on this plot
	qwtfigure(10);
	qwtplot2(circleData_x1, circleData_y1, nc, "circle", "-qm", 1, 12, circleTime_1);
	qwttitle("'top view' test");

	
#ifdef USEMARBLE
	// --------   map view test: draw on a map  ---------------------
	t4[0] = sinusTime_1[0]; t4[1] = sinusTime_1[int(n1 / 4.)];
	 t4[2] = sinusTime_1[int(n1 / 3.)]; 
	t4[3] = sinusTime_1[int(n1 / 2.)]; 
	topview(18);
	qwtplot2(east, north, mwn, "test mw", "-rb", 2,  8, t4);
	qwttitle("'marble' top view");
#endif

end:;
	//  --------- show 'main control window' ----------
	//  this is very useful when you have  about 20 plots 
	qwtshowmw();
		

	std::cout << "press a key to close the DLL:" << std::endl;
	int  ch = getch();


	delete[] x_sinusData_2;
	delete[] y_sinusData_2;
	delete[] x4;
	delete[] y4;
	delete[] y5;
	delete[] y6;
	delete[] x3d;
	return 0;
}

void createInfo() {
	int i;
	x_sinusData_2 = new double[n3];
	y_sinusData_2 = new double[n3];
	x4 = new double[n4];
	y4 = new double[n4];
	y5 = new double[n4];
	y6 = new double[n4];

	delta = x_sinusData_2[n3 - 1] - x_sinusData_2[0];

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




