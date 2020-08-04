#ifndef TOPVIEWLINEOBJECT_H
#define TOPVIEWLINEOBJECT_H

#include "MapGraphics_global.h"
#include "Position.h"
#include "MapGraphicsObject.h"
#include <QPointF>
#include <list>

struct LineItemInfo;
class QPointF;

class TopViewLineObject : public MapGraphicsObject
{
    Q_OBJECT
public:
	double llaOrigin[3];

    explicit TopViewLineObject(LineItemInfo* line_, double* origin_, 
                        MapGraphicsObject *parent = 0);
    virtual ~TopViewLineObject();

    //pure-virtual from MapGraphicsObject
    QRectF boundingRect() const;

    //pure-virtual from MapGraphicsObject
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

  //  qreal thickness() const;
  //  void setThickness(qreal nThick);
    
signals:
    
public slots:
   // void setEndPointA(const Position& pos);
   // void setEndPointB(const Position& pos);
   // void setEndPoints(const Position& a,
   //                   const Position& b);

//private slots:
    //void updatePosition();

private:
   // Position _a;
   // Position _b;
  //  qreal _thickness;
	LineItemInfo* line;
	double  origin[3];
	struct Point2{
		double x, y;
		Point2() : x(0.), y(0.) {}
		Point2(double x_, double y_): x(x_), y(y_) {}
		double dist(const Point2& p) { return ((x-p.x)*(x-p.x) + (y-p.y)*(y-p.y)); }
	};
    std::list<Point2> enuPoints;
	Point2 minb, maxb;

	//QPointF center, minF, maxF;
	QPointF* points;
	int pointSize;

	void processPoints();
};

#endif // TOPVIEWLINEOBJECT_H
