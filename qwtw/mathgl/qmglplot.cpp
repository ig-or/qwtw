

#include "xstdef.h"
#include "sfigure.h"
#include "xmutils.h"

#include <QLayout>
#include <QLabel>
#include "line.h"
#include "qmglplot.h"
#include "qmgl.h"

int xmprintf(int level, const char * _Format, ...);


QMglPlot::QMglPlot(const std::string& key_, XQPlots* pf_, QWidget *parent) : qmgl(0), JustAplot(key_, pf_, parent, jMathGL) {

}

QMglPlot::~QMglPlot() {

}

int QMglPlot::qInit() {
	xmprintf(6, "QMglPlot::qInit() starting\n");
	if (qmgl != 0) return 1;
	xmprintf(6, "QMglPlot::qInit() creating QMGL..\n");
	qmgl = new QMGL1(this);
	xmprintf(6, "QMglPlot::qInit() OK1\n");

	// Create a label to show the geodetic position
	//QLabel * positionLabel = new QLabel();
	//positionLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	// Add all widgets to the vertical layout.
	QVBoxLayout *layout = new QVBoxLayout;

	layout->setSpacing(1); layout->setMargin(1);
	layout->addWidget(qmgl);
	//layout->addWidget(positionLabel);

	setLayout(layout);
	resize(500, 400);

	show();
	xmprintf(6, "QMglPlot::qInit() done\n");
	return 0;
}

void QMglPlot::xlabel(const std::string&  s) {

}
void QMglPlot::ylabel(const std::string&  s) {

}
void QMglPlot::zlabel(const std::string&  s) {

}


void QMglPlot::addLine(LineItemInfo* line) {
    if (qmgl == 0) return;
    xmprintf(6, "\t\tQMglPlot::addLine start \n");
	JustAplot::addLine(line);
    xmprintf(6, "\t\tQMglPlot::addLine .. \n");
    //qmgl->addLine(line->size, line->x, line->y, line->z, line->style);
	qmgl->addLine(line);
    xmprintf(6, "\t\tQMglPlot::addLine finish \n");
}


void QMglPlot::addMesh(const MeshInfo& info) {
        qmgl->addSurf(info);
}

/*
void MarView::drawMarker(double t) {
	JustAplot::drawMarker(t);
	mw->drawMarker(t);
}
*/
