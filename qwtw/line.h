/**
	plot line definition
*/

#pragma once

#include <string>
#include <list>

class SQWLine;

/**
one single line on a plot.
*/
struct LineItemInfo {
	double* x = nullptr;
	double* y = nullptr;
	double* z = nullptr;
	/** just a reference, supposed to be common for all the figures
	   if "0", then reference maybe "x"
	*/
	double* time = nullptr;
	/**
	0 - do not use markers
	1 - draw 'vertical line' markers using "x" (y[i] - y[i-1] ==  const), regular grid
	2 - draw 'vertical line' markers using "x" (y[i] - y[i-1] !=  const), not regular grid
	3 - draw 'point' marker using "time" info
	*/
	int mode = 0;
	bool important; ///< if 'false', it will not participate in 'clipping'
	long long size; ///< size for x, y, z , and time
	std::string  legend;
	std::string  style;
	int lineWidth;
	int symSize;  ///< symbol size
	SQWLine* sqwLine; 
	bool ok;  ///<   true if this line is good

	/* we will put here the indeces of the points with about 1m between*/
	std::list<unsigned int> smallCoordIndex;
	int id;

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
	bool lCheck();
};



