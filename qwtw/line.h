/**
	plot line definition
*/

#pragma once

#include <string>
#include <list>

class SQWLine;
class QwtPlotCurve;
class QwtPlotMarker;

/**
one single line on a plot.
*/
struct LineItemInfo {
	double* x;
	double* y;
	double* z;
	/** just a reference, supposed to be common for all the figures
	   if "0", then reference maybe "x"
	*/
	double* time;
	/**
	0 - do not use markers
	1 - draw 'vertical line' markers using "x" (y[i] - y[i-1] ==  const), regular grid
	2 - draw 'vertical line' markers using "x" (y[i] - y[i-1] !=  const), not regular grid
	3 - draw 'point' marker using "time" info
	*/
	int mode;
	bool important; ///< if 'false', it will not participate in 'clipping'
	size_t size; ///< size for x, y, z , and time
	std::string  legend;
	std::string  style;
	int lineWidth;
	int symSize;  ///< symbol size
	SQWLine* sqwLine; 
	bool ok;  ///<   true if this line is good

	/* we will put here the indeces of the points with about 1m between*/
	std::list<unsigned int> smallCoordIndex;

	struct Marker {
		bool active;
		long long index;
		double time; ///< ?
		Marker() : active(false), index(0) { time = 0.0; }
	};
	Marker ma;

	/**  create 2D line */
	LineItemInfo(double* x_, double* y_, size_t size_, std::string legend_,
		int mode_ = 1, double* time_ = 0);

	/**  create 3D line */
	LineItemInfo(double* x_, double* y_, double* z_, size_t size_, std::string legend_,
		double* time_ = 0);
	~LineItemInfo();
};


struct FigureItem {
	QwtPlotCurve* line;
	LineItemInfo* info;
	//long long key;

	QwtPlotMarker* ma;
	FigureItem(LineItemInfo* info_, QwtPlotCurve* line_);
	~FigureItem();
};



