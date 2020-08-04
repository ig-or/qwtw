

#include "justaplot.h"
#include "sfigure.h"
#include "sqwlinetree.h"
#include "xmutils.h"
#include "xmatrixplatform.h"
#include "xmatrix2.h"

#define REMEMBER_EVERYTHING

LineItemInfo::LineItemInfo(double* x_, double* y_, size_t size_, std::string legend_, 
	   int mode_, double* time_) {
    mxassert(size_ > 0, "");
    x = x_;
    y = y_;
	z = 0; //  2D plot
    time = time_;
    important = true;

#ifdef REMEMBER_EVERYTHING

    if(x_ != 0) {
	    x = new double[size_]; memcpy(x, x_, size_*sizeof(double));
    }
    if(y_ != 0) {
	    y = new double[size_]; memcpy(y, y_, size_*sizeof(double));
    }
    if(time_ != 0) {
	    time = new double[size_]; memcpy(time, time_, size_*sizeof(double));
    } 
#endif

	mode = mode_;
	size = size_;
	legend = legend_;

	style = std::string();
	lineWidth = 0; // default value ?????
	symSize = 8;

	if(mode == 3) {
	    sqwLine = new SQWLine(5);
	    sqwLine->onLine(x, y, size);
	} else {
	    sqwLine = 0;
	}
}

LineItemInfo::LineItemInfo(double* x_, double* y_, double* z_, size_t size_, std::string legend_,
	double* time_) {

	mxassert(size_ > 0, "");
	x = x_;
	y = y_;
	z = z_; //  3D plot
	time = time_;
	important = true;

#ifdef REMEMBER_EVERYTHING

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
#endif
	size = size_;
	legend = legend_;


	if (time_ == 0) {
		mode = 2;
	} else {
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


	style = std::string();
	lineWidth = 0; // default value ?????
	symSize = 8;
	sqwLine = 0;
}

LineItemInfo::~LineItemInfo() {
#ifdef REMEMBER_EVERYTHING
	if(x != NULL) delete[] x;
	if(y != NULL) delete[] y;
	if (z != NULL) delete[] z;
	if(time != NULL) delete[] time;
#endif

    if(sqwLine != 0) {
	   delete sqwLine;
    }
}

long long fcp(LineItemInfo* i, double x, double y) {
	long long ret = 0xffffffff;
	if ((i == 0) || (i->mode == 0)) return ret;
	if (i->mode == 3) {
		mxat(i->sqwLine != 0);
		ret = i->sqwLine->findClosestPoint(x, y);
		mxat(ret < 0xffffffff);
	} else {
		ret = findClosestPoint_1(0, i->size - 1, i->x, x);
	}
	return ret;
}

double findDistance(LineItemInfo* i, double x, double y, long long& index) {
	double  ret = BIGNUMBER;
	index = fcp(i, x, y);
	if (index < 0xffffffff) {
		IMatrix<2, 1> v(x - i->x[index], y - i->y[index]);
		ret = v.length();
	}
	return ret;
}

JustAplot::JustAplot(const std::string& key_, XQPlots* pf_, QWidget *parent, int type_) : QDialog(parent,
                                Qt::Window | Qt::WindowTitleHint | Qt::WindowSystemMenuHint |
				Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint) {
	key = key_;
	cInfo = 0;
	pf = pf_;
	name = key;
	type = type_;

	setAttribute(Qt::WA_DeleteOnClose);
}

void JustAplot::addLine(LineItemInfo* line) {
	linesInfo.push_back(line);
	cInfo = line;
}

JustAplot::~JustAplot() {
	std::list<LineItemInfo*>::iterator it;
	for (it = linesInfo.begin(); it != linesInfo.end(); it++) {
		LineItemInfo* i = *it;
		delete i;
	}
	linesInfo.clear();
}

void JustAplot::title(const std::string& s) {
	long long n = 0;
	for (std::list<LineItemInfo*>::iterator it = linesInfo.begin(); it != linesInfo.end(); it++) {
		LineItemInfo* i = *it;
		n += i->size;
	}
	char np[64]; sprintf(np, " (%lld points)", n);
	std::string tmp = s;
	tmp.append(np);

	setWindowTitle(tmp.c_str());
	name = s;
}

 void JustAplot::footer(const std::string& s) {
	
}

void JustAplot::makeMarkersVisible(bool visible) {
	if (linesInfo.empty()) return;

	for (std::list<LineItemInfo*>::iterator it = linesInfo.begin(); it != linesInfo.end(); it++) {
		LineItemInfo* i = *it;
		if (i->mode == 0) continue;
		i->ma.active = visible;
	}
 }

 void JustAplot::drawMarker(double t) {
	 if (linesInfo.empty()) return;

	 for (std::list<LineItemInfo*>::iterator it = linesInfo.begin(); it != linesInfo.end(); it++) {
		 LineItemInfo* i = *it;
		 if (i->mode == 0) continue;
		 i->ma.active = true;
		 i->ma.time = t;

		 switch (i->mode) {
			 case 1:
			 case 2:
				 i->ma.index = findClosestPoint_1(0, i->size - 1, i->x, t);

				 break;
			 case 3:
				 i->ma.index = findClosestPoint_1(0, i->size - 1, i->time, t);

				 break;
		 };
	 }

 }

 void JustAplot::onClip(double t1, double t2) {


 }
