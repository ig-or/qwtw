

#include <iostream>
#include "line.h"
#include "sqwlinetree.h"
#include "xmatrix2.h"
#include "xstdef.h"

int xmprintf(int level, const char* _Format, ...);

LineItemInfo::LineItemInfo(double* x_, double* y_, size_t size_, std::string legend_,
		int mode_, double* time_) {
	mxat(size_ > 0);
	if ((size_ < 1) || (x_ == 0) || (y_ == 0)) {
		x = 0;
		y = 0;
		z = 0; 
		time = 0;
		id = -187;
		ok = false;
		xmprintf(0, "ERROR: LineItemInfo::LineItemInfo: cannot create line [%s]; size = %d\n", 
			legend_.c_str(), size);
		return;
	}
	ok = true;

	x = x_;
	y = y_;
	z = 0; //  2D plot
	time = time_;
	important = true;

	if (x_ != 0) {
		x = new double[size_]; 
		memcpy(x, x_, size_ * sizeof(double));
	}
	if (y_ != 0) {
		y = new double[size_]; 
		memcpy(y, y_, size_ * sizeof(double));
	}
	if (time_ != 0) {
		time = new double[size_]; 
		memcpy(time, time_, size_ * sizeof(double));
	}

	mode = mode_;
	size = size_;
	legend = legend_;

	style = std::string();
	lineWidth = 0; // default value ?????
	symSize = 8;

	if (mode == 3) {
		sqwLine = new SQWLine(5);
		sqwLine->onLine(x, y, size);
	}
	else {
		sqwLine = 0;
	}
	id = 0;
}

LineItemInfo::LineItemInfo(double* x_, double* y_, double* z_, size_t size_, std::string legend_,
	double* time_) {

	xmprintf(7, "\t\tLineItemInfo::LineItemInfo start \n");

	mxassert(size_ > 0, "");

	if ((size_ < 1) || (x_ == 0) || (y_ == 0) || (z_ == 0)) {
		x = 0;
		y = 0;
		z = 0;
		time = 0;
		id = -188;
		ok = false;
		xmprintf(0, "ERROR: LineItemInfo::LineItemInfo: cannot create line [%s]; size = %d\n",
			legend_.c_str(), size);
		return;
	}
	ok = true;

	x = x_;
	y = y_;
	z = z_; //  3D plot
	time = time_;
	important = true;

	if (x_ != 0) {
		x = new double[size_]; memcpy(x, x_, size_ * sizeof(double));
	}
	if (y_ != 0) {
		y = new double[size_]; memcpy(y, y_, size_ * sizeof(double));
	}
	if (z_ != 0) {
		z = new double[size_]; memcpy(z, z_, size_ * sizeof(double));
	}
	if (time_ != 0) {
		time = new double[size_]; memcpy(time, time_, size_ * sizeof(double));
	}
	xmprintf(7, "\t\tLineItemInfo::LineItemInfo xyzt created \n");
	size = size_;
	legend = legend_;


	if (time_ == 0) {
		mode = 2;
	}
	else {
		mode = 3;

		//  fill in smallCoordIndex:
		unsigned int ip = 0;
		smallCoordIndex.push_back(ip);
		for (unsigned int i = 1; i < size; i++) {
			V3 v(x[i] - x[ip], y[i] - y[ip], z[i] - z[ip]);
			if (v.norma2() > ONE) {
				ip = i;
				smallCoordIndex.push_back(ip);
			}
		}
		smallCoordIndex.push_back(size - 1);
		std::cout << "LineItemInfo(): smallCoordIndex.size() = " << smallCoordIndex.size() << std::endl;
	}
	xmprintf(7, "\t\tLineItemInfo::LineItemInfo almost \n");

	style = std::string();
	lineWidth = 0; // default value ?????
	symSize = 8;
	sqwLine = 0;
	id = 0;
	xmprintf(7, "\t\tLineItemInfo::LineItemInfo done  \n");
}

LineItemInfo::~LineItemInfo() {
	if (x != NULL) delete[] x;
	if (y != NULL) delete[] y;
	if (z != NULL) delete[] z;
	if (time != NULL) delete[] time;

	if (sqwLine != 0) {
		delete sqwLine;
	}
	
}

bool LineItemInfo::lCheck() {
	if (x == 0) { return false; }
	if (y == 0) { return false; }
	if (size <= 0) { return false; }
	if (lineWidth < 0) { return false; }
	if (symSize < 0) { return false;  }
	if (!ok) { return false;  }

	return true;
}


