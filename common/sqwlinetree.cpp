
/**  lines parser.

\file     sqwlinetree.cpp
\author   Igor Sandler

\version 1.0

*/



#include "sqwlinetree.h"
#include "xstdef.h"
#include "xmatrix2.h"

LineSegment::LineSegment(size_t i1_, size_t i2_) {
	i1 = i1_;
	i2 = i2_;
}

SQWLineItem::SQWLineItem(int level_, SQWLine* line_, double x1_, double x2_, double y1_, double y2_, SQWLineItem* parent_) {
	parent = parent_;
	level = level_;
	line = line_;
	empty = true;
	checked = false;
	
	x1 = x1_; y1 = y1_; x2 = x2_; y2 = y2_; 
	cx = (x1 + x2) / 2.;  
	cy = (y1 + y2) / 2.;

	if (level == line->maxLevel) {
		low[0][0] = 0; low[0][1] = 0; 
		low[1][0] = 0; low[1][1] = 0; 

		line->addLowLevelItem(cx, cy, this);
	} else {
		low[0][0] = new SQWLineItem(level+1, line, x1, cx, y1, cy, this);       
		low[0][1] = new SQWLineItem(level+1, line, cx, x2, y1, cy, this);
		low[1][0] = new SQWLineItem(level+1, line, x1, cx, cy, y2, this);
		low[1][1] = new SQWLineItem(level+1, line, cx, x2, cy, y2, this);
	}
}

SQWLineItem::~SQWLineItem() {
	if (level != line->maxLevel) {
		delete low[0][0];
		delete low[0][1];
		delete low[1][0];
		delete low[1][1];
	}
}

void SQWLineItem::addPoint(size_t i) {
	if (empty) {
		empty = false;
		i1 = i;
		i2 = i;
	} else {  //   start of saving
		if (i == (i2 + 1)) {  //  continue this line segment:
			i2++;
		} else { //  start of next segment
			//  finish previous segment:
			LineSegment ls(i1, i2);
			segments.push_back(ls);

			//  start a new one:
			i1 = i;
			i2 = i;
		}
	}

	// feed the child:
	if (level != line->maxLevel) {
		if (line->x[i] < cx) {
			if (line->y[i] < cy) {
				low[0][0]->addPoint(i);
			} else {
				low[1][0]->addPoint(i);
			}
		} else {
			if (line->y[i] < cy) {
				low[0][1]->addPoint(i);
			} else {
				low[1][1]->addPoint(i);
			}
		}
	}
}

void SQWLineItem::polish() {
	if (!empty) {
		LineSegment ls(i1, i2);
		segments.push_back(ls);

		if (level != line->maxLevel) {
			low[0][0]->polish(); 		low[0][1]->polish(); 
			low[1][0]->polish(); 		low[1][1]->polish(); 
		}
	}
}

size_t SQWLineItem::findClosestPoint(double xx, double yy, double& distance) {
	mxassert(!empty, "");
	double md1;
	distance =  sqr(xx - x1) + sqr(yy - y1);
	if ((md1 = sqr(xx - x1) + sqr(yy - y2)) > distance) {
		distance = md1;
	}
	if ((md1 = sqr(xx - x2) + sqr(yy - y1)) > distance) {
		distance = md1;
	}
	if ((md1 = sqr(xx - x2) + sqr(yy - y2)) > distance) {
		distance = md1;
	}
	std::list<LineSegment>::iterator it = segments.begin();
	size_t i, ret = 0xffffffff;
	while (it != segments.end()) {
		for (i = it->i1; i <= it->i2; i++) {
			if ((md1 = sqr(xx - line->x[i]) + sqr(yy - line->y[i])) < distance) {
				distance = md1;
				ret = i;
			}
		}
		it++;
	}
	return ret;
}


SQWLine::SQWLine(int maxLevel_) : maxLevel(maxLevel_), 	sqw(0), low(0)	{
	x = 0; y = 0; w = 0;
	nn = 1 << maxLevel;
}

SQWLine::~SQWLine() {
	if (sqw != 0) {
		delete sqw;
	}
}

void SQWLine::onLine(double* x_, double* y_, size_t w_) {
	x = x_; y = y_;
	w = w_;
	mxassert(w > 0, "");
	if (w  < 1) {
		return;
	}

	//  find min and max;
	x1 = x[0];
	x2 = x[w-1];
	y1 = y[0];
	y2 = y[w-1];
	size_t i;
	for (i = 1; i < w; i++) {
		if (x1 > x[i]) x1 = x[i];
		if (y1 > y[i]) y1 = y[i];
		if (x2 < x[i]) x2 = x[i];
		if (y2 < y[i]) y2 = y[i];
	}

	if((x2 - x1) <= DELTA_ZERO_1) {
	    mxassert(false, "SQWLine::onLine: empty line #1");
	    return;
	}
	if((y2 - y1) <= DELTA_ZERO_1) {
	    mxassert(false, "SQWLine::onLine: empty line #2");
	    return;
	}


	//  remove old info:
	if (sqw != 0) {
		delete sqw;
	}
	if (low != 0) { 
		delete low; 
	}
	dx = (x2 - x1) / nn;
	dy = (y2 - y1) / nn;

	low = new SQWLineItem*[nn*nn];
	memset(low, 0, sizeof(SQWLineItem*) * nn * nn);

	//  create new:
	sqw = new SQWLineItem(0, this, x1, x2, y1, y2);

	//  add points:
	for (i = 0; i < w; i++) {
		sqw->addPoint(i);
	}
	sqw->polish();
}

void SQWLine::addLowLevelItem(double cx, double cy, SQWLineItem* item) {
	mxassert(low != 0, "");
	if (low == 0) return;
	int i = floor(((cx - x1) / dx));
	int j = floor(((cy - y1) / dy));
	mxassert((i >= 0) && (i < nn), "");
	mxassert((j >= 0) && (j < nn), "");
	mxassert(low[i + nn*j] == 0, "");
	low[i+nn*j] = item;
}

size_t SQWLine::findClosestPoint(double xx, double yy) {
	double ds = (dx < dy) ? dx : dy;
	int i, j;
	size_t ii = 0xffffffff, iTest;
	double cDistance;
	bool find = false;

	for (i = 0; i < nn*nn; i++) {
		low[i]->checked = false;
	}

	//  find max distance:
	double md =  sqr(xx - x1) + sqr(yy - y1), md1;
	if ((md1 = sqr(xx - x1) + sqr(yy - y2)) > md) {
		md = md1;
	}
	if ((md1 = sqr(xx - x2) + sqr(yy - y1)) > md) {
		md = md1;
	}
	if ((md1 = sqr(xx - x2) + sqr(yy - y2)) > md) {
		md = md1;
	}
	cDistance = md;

	double r = 0., f, df, cx, cy;

	while ((r <= md) && (!find)) {
		f = 1e-4;
		df = (r > 0.5*ds) ? asin(ds / r) : 2.0*pii;
		while (f < 2.*pii) {
			cx = xx + r*sin(f);
			cy = yy + r*cos(f);
			f += df;
			i = floor(((cx - x1) / dx));
			j = floor(((cy - y1) / dy));
			if ((i < 0) || (i >= nn) || (j < 0) || (j >= nn)) {
				continue;
			}
			if ((low[i + nn*j]->checked) || (low[i + nn*j]->empty)) {
				continue;
			}
			if (r > 0.5*ds) find = true;
			low[i + nn*j]->checked = true;

			iTest = low[i + nn*j]->findClosestPoint(xx, yy, md1);
			if (md1 < cDistance) {
				cDistance = md1;
				ii = iTest;
			}
		}
		r += ds;
	}
	return ii;
}



