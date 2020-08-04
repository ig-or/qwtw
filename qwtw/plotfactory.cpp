
/**

	Main library interface file.
	Wrapper for all the functions.
	

	\file plotfactory.cpp
	\author   Igor Sandler
	\date    Jul 2009
	\version 1.0
	
*/



#include "plotfactory.h"
#include "sfigure.h"
#include "figure2.h"
//#include "figure3.h"



PlotFactory::PlotFactory(QWidget * parent1) {
	gra = new Gra1(parent1);
	lastFigureType  =  0;
	connect(gra, SIGNAL(selection(int, int)), this, SLOT(selection(int, int)));
}

PlotFactory::~PlotFactory()		{
	delete gra;
}


void PlotFactory::figure(int n)  {
	gra->figure(n);  lastFigureType  =  2;
}

/*
void PlotFactory::figure3(int n)  {
	gra->figure3(n); lastFigureType  =  3;
}
*/

void PlotFactory::onExit() {
	gra->onExit();
}

void PlotFactory::title(char* s) {
	switch (lastFigureType) {
	case 2:
		if (gra->cf != 0) {    
			gra->cf->title(s);
		} else {

		}
	break;
	case 3:
		/*
		if (gra->cf3 != 0) {    
			gra->cf3->title(s);
		} else {

		}
		*/
		break;
	};
}

void PlotFactory::legend(char* text, double min, double max, int orientation) {
/*	switch (lastFigureType) {
	case 3:
		if (gra->cf3 != 0) {    
			gra->cf3->legend(text, min, max, orientation);
		} else {

		}
		break;
	};
	*/
}

void PlotFactory::xlabel(char* s) {
	switch (lastFigureType) {
	case 2:
		if (gra->cf != 0) {    
			gra->cf->xlabel(s);
		} else {

		}
		break;
	case 3:
		/*
		if (gra->cf3 != 0) {    
			gra->cf3->xlabel(s);
		} else {

		}
		*/
		break;
	};
}
void PlotFactory::ylabel(char* s) {
	switch (lastFigureType) {
	case 2:
		if (gra->cf != 0) {    
			gra->cf->ylabel(s);
		} else {

		}
		break;
	case 3:
		/*
		if (gra->cf3 != 0) {    
			gra->cf3->ylabel(s);
		} else {

		}
		*/
		break;
	};
}
void PlotFactory::zlabel(char* s) {
	switch (lastFigureType) {
	case 3:
		/*
		if (gra->cf3 != 0) {    
			gra->cf3->zlabel(s);
		} else {

		}
		*/
		break;
	};
}

void PlotFactory::plot(double* x, double* y, int size, char* name, const char* style, int lineWidth, int symSize) {
	if (gra->cf != 0) {    
		gra->cf->plot(x, y, size, name, style, lineWidth, symSize);
	} else {

	}
}

/*
void PlotFactory::plot3(double* p, int N, double res, double* r, double* c) {
	if (gra->cf3 != 0) {    
		gra->cf3->plot(p, N, res, r, c);
	} else {

	}
}
*/
void PlotFactory::selection(int ha, int index) {
	emit select(ha, index);
}