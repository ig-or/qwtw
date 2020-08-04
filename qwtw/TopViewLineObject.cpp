#include "TopViewLineObject.h"

#include "guts/Conversions.h"
#include <QtGlobal>
#include "justaplot.h"
#include "xmcoords.h"

TopViewLineObject::TopViewLineObject(LineItemInfo* line_,  double* origin_, MapGraphicsObject *parent) :
        MapGraphicsObject(false, parent), line(line_)  {
   // _thickness = qBound<qreal>(0.0, thickness, 5.0);
			pointSize = 0;
			points = 0;
	memcpy(origin, origin_, 3*sizeof(double));
	processPoints();
//    updatePosition();
	setZValue(10000000.);
}

TopViewLineObject::~TopViewLineObject(){
	if (points != 0) {
		delete[] points;
	}


}

void TopViewLineObject::processPoints() {
	double D = 1.0 *  1.0;

	size_t i, k;
	std::list<Point2> tmp;
	k = 0;
	Point2 p(line->x[0], line->y[0]);
	Point2 p1;
	tmp.push_back(p);
	
	for (i = 1; i < line->size; i++) {
		p1.x = line->x[i];    p1.y = line->y[i];
		if (p.dist(p1) > D) { // we need to save 'i-1' point:
			if (i > (k+1)) {
				i--;    //  now may be i == k+1
			} 
			k = i;
			p.x = line->x[k];    p.y = line->y[k];	
			tmp.push_back(p);
		}
	}
	// do not forget about end point:
	if (k < (line->size - 1)) {
		tmp.push_back(Point2(line->x[line->size-1], line->y[line->size-1]));
	}

	enuPoints.clear();
	std::list<Point2>::iterator it;
	minb = Point2(line->x[0], line->y[0]); 
	maxb = Point2(line->x[0], line->y[0]); 
	for(it = tmp.begin(); it != tmp.end(); it++) {


		p = *it;
		enuPoints.push_back(p);
		if (p.x < minb.x) minb.x = p.x; 
		if (p.y < minb.y) minb.y = p.y; 
		if (p.x > maxb.x) maxb.x = p.x; 
		if (p.y > maxb.y) maxb.y = p.y; 

	}
	//center.x = (minb.x + maxb.x) * 0.5;
	//center.y = (minb.y + maxb.y) * 0.5;

	if (points != 0) {
		delete[] points;
	}

	ecef2LLA(origin, llaOrigin);
	ENUframe fa;   fa.setOrigin(origin);
	Vector3 center((minb.x + maxb.x) * 0.5, (minb.y + maxb.y) * 0.5, llaOrigin[2]);
	Vector3 llaCenter = fa.getLLACoord(center);
	

	setPos(QPointF(llaCenter[1]*Rad2Deg, llaCenter[0]*Rad2Deg));
	posChanged();

	int index = 0;
	pointSize = enuPoints.size();
	points = new QPointF[pointSize];
	for(it = enuPoints.begin(); it != enuPoints.end(); it++) {
		points[index] = QPointF(it->x, it->y);

		index++;
	};

}

//pure-virtual from MapGraphicsObject
QRectF TopViewLineObject::boundingRect() const{
	QRectF ret;
	double x = (fabs(minb.x) < fabs(maxb.x)) ?  fabs(maxb.x) : fabs(minb.x);
	double y = (fabs(minb.y) < fabs(maxb.y)) ?  fabs(maxb.y) : fabs(minb.y);

    const QRectF toRet(-x,  -y,  x,  y);
    return toRet;
}

//pure-virtual from MapGraphicsObject
void TopViewLineObject::paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing, false);
    QPen pen = painter->pen();
    pen.setWidthF(0);
	pen.setColor(Qt::red);
    painter->setPen(pen);

	painter->drawPolyline(points, pointSize);

#ifdef _DEBUG
	xm_printf("p");
#endif
}


//void TopViewLineObject::setThickness(qreal nThick)
//{
 //   _thickness = qBound<qreal>(0.0, nThick, 5.0);
 //   this->redrawRequested();
//}



//public slot 
/*
void TopViewLineObject::setEndPoints(const Position &a,
                              const Position &b)
{
    _a = a;
    _b = b;
    this->updatePositionFromEndPoints();
    this->redrawRequested();
}
*/
//private slot
/*
void TopViewLineObject::updatePosition()
{
    const qreal avgLon = (_a.longitude() + _b.longitude()) / 2.0;
    const qreal avgLat = (_a.latitude() + _b.latitude()) / 2.0;
    this->setPos(QPointF(avgLon, avgLat));
}
*/