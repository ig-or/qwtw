

#include "justaplot.h"
#include "line.h"
#include "sqwlinetree.h"
#include "sfigure.h"
#include "xmutils.h"
#include "xmatrixplatform.h"
#include "xmatrix2.h"


long long fcp(LineItemInfo* i, double x, double y) {
	long long ret = 0xffffffff;
	if ((i == 0) || (i->mode == 0)) return ret;
	if (i->mode == 3) {
		mxat(i->sqwLine != 0);
		ret = i->sqwLine->findClosestPoint(x, y);
		//mxat(ret < 0xffffffff);
	} else {
		mxat(i->size > 0);
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

JustAplot::JustAplot(const std::string& key_, XQPlots* pf_, QWidget *parent, JPType type_) : QDialog(parent,
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

void JustAplot::removeLine(LineItemInfo* line) {
	std::list<LineItemInfo*>::iterator it;
	for (it = linesInfo.begin(); it != linesInfo.end(); it++) {
		LineItemInfo* i = *it;
		if (i == line) {
			delete i;
			linesInfo.erase(it);
			break;
		}
	}

	//  update cInfo
	if (linesInfo.empty()) {
		cInfo = 0;
	}	else {
		if (cInfo == line) {
			cInfo = linesInfo.back();
		}
	}
}

JustAplot::~JustAplot() {
	emit exiting(key);
	
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
	char np[64]; snprintf(np, 64, " (%lld points)", n);
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
				 mxat(i->size > 0);
				 i->ma.index = findClosestPoint_1(0, i->size - 1, i->x, t);

				 break;
			 case 3:
				 mxat(i->size > 0);
				 i->ma.index = findClosestPoint_1(0, i->size - 1, i->time, t);

				 break;
		 };
	 }

 }

 void JustAplot::onClip(double t1, double t2) {


 }
