
/**  lines parser.

\file     sqwlinetree.h
\author   Igor Sandler

\version 1.0

*/



#ifndef SQWLineTREE_H_FILE
#define SQWLineTREE_H_FILE

#include <list>
#include "xstdef.h"

/**   part of a line.

*/
struct LineSegment {
	size_t i1; ///< start point index
	size_t i2; ///< end point index
	LineSegment(size_t i1_, size_t i2_);
};

class SQWLine;

/**   a square which can contain line segments  
*/
class SQWLineItem {
public:
	SQWLineItem* parent; ///< parent (upper level) square
	SQWLineItem*  low[2][2]; ///< low level squares
	int level;  ///< this square level number
	bool empty; ///< do we have any line segments inside? 
	bool checked;
	SQWLine* line; ///< our host object (who created us)
	std::list<LineSegment> segments; ///< all the segments
	double x1, x2, y1, y2; ///< borders
	double cx, cy; ///< center
	
	/**
	   @param[in] line_ host object
	*/
	SQWLineItem(int level_, SQWLine* line_, double x1_, double x2_, double y1_, double y2_, SQWLineItem* parent_ = 0);
	~SQWLineItem();

	/** add point to the tree
	*/
	void addPoint(size_t i);
	void polish();

	/**
	   @param[out] distance minimum distance from input point 
	   @return closest point index or 0xffffffff  if no points found
	*/
	size_t findClosestPoint(double xx, double yy, double& distance);
private:
	size_t i1;
	size_t i2;
};

/** an object for line handling (line preprocessor)
    it contains some information which is common for all 'line items'
*/
class SQWLine {
public:
     /**
	@param[in] maxLevel_ maximum depth level
	*/
	SQWLine(int maxLevel_);
	~SQWLine();

	/** set line info. init everything. time consuming operation.

	*/
	void onLine(double* x_, double* y_, size_t w_);
	/**
	   @return closest point index ?
	*/
	size_t findClosestPoint(double xx, double yy);
	friend SQWLineItem;
private:
     int maxLevel;
     int nn; ///< how many small squares we will have on one side of big square
	SQWLineItem* sqw;  ///< out top item
	SQWLineItem** low;  ///< low level items

	// our line:
	double* x;
	double* y;
	size_t	w;

	double x1, x2, y1, y2;
	double dx, dy;  ///< sides of the small squares
	void addLowLevelItem(double cx, double cy, SQWLineItem* item);
};

#endif

