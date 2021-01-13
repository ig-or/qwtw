

#include "xstdef.h"
#include "sfigure.h"
#include "xmutils.h"

#include <QLayout>
#include <QLabel>
#include "qmglplot.h"
#include "qmgl.h"

int xmprintf(int level, const char * _Format, ...);


QMglPlot::QMglPlot(const std::string& key_, XQPlots* pf_, QWidget *parent) : JustAplot(key_, pf_, parent, jMathGL) {
    
}

QMglPlot::~QMglPlot() {

}

int QMglPlot::qInit() {
	if (qmgl != 0) return 1;
	qmgl = new QMGL1(this);

	// Create a label to show the geodetic position
	//QLabel * positionLabel = new QLabel();
	//positionLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	// Add all widgets to the vertical layout.
	QVBoxLayout *layout = new QVBoxLayout;

	layout->setSpacing(1); layout->setMargin(1);
	layout->addWidget(qmgl);
	//layout->addWidget(positionLabel);

	setLayout(layout);
	resize(400, 300);

	show();
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
    qmgl->addLine(line->size, line->x, line->y, line->z, line->style);
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
