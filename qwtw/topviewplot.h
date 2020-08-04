#ifndef TOPVIEWPLOT_H_FILE
#define TOPVIEWPLOT_H_FILE

#include <Qt>
#include <QDialog>
#include <QList>
#include "justaplot.h"

class TopViewLineObject;
class MapGraphicsScene;
class MapGraphicsView;

class Ui_topviewplotdlg;

class TopViewPlot: public JustAplot {
	Q_OBJECT
public:
	TopViewPlot(const std::string& key_, QWidget *parent);
	~TopViewPlot();
	virtual void xlabel(const std::string&  s);
	virtual void ylabel(const std::string&  s);
	virtual void drawMarker(double X, double Y, int type = 1);
	virtual void addLine(LineItemInfo* line);
	void setEcefOrigin(double* origin);

private:
	std::list<TopViewLineObject*> lines;
	Ui_topviewplotdlg* ui;
	double ecefOrigin[3]; ///<  for conversion between ENU and ECEF frames

	MapGraphicsScene* scene;
	MapGraphicsView* view;
};


#endif

