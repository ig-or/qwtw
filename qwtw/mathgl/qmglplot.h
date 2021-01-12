#pragma once

#include "xstdef.h"
#include "justaplot.h"

class QMGL1;



class QMglPlot: public JustAplot {
	Q_OBJECT

public:
	QMglPlot(const std::string& key_, XQPlots* pf_, QWidget *parent);
	virtual ~QMglPlot();

	int qInit();

	void xlabel(const std::string&  s);
	void ylabel(const std::string&  s);
    void zlabel(const std::string&  s);

	virtual void drawMarker(double X, double Y, int type = 1) {}

	//virtual void drawMarker(double X, double Y, int type = 1) {};
	//virtual void drawMarker(double t);
//protected:
	void addLine(LineItemInfo* line);
    void addMesh(const MeshInfo& info);

private:
	QMGL1* qmgl;

};
