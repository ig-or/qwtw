


#include "topviewplot.h"
#include "ui_topviewmap.h"
#include "MapGraphicsView.h"
#include "MapGraphicsScene.h"
#include "tileSources/GridTileSource.h"
#include "tileSources/OSMTileSource.h"
#include "tileSources/CompositeTileSource.h"
#include "xstdef.h"
#include "TopViewLineObject.h"
#include "xmcoords.h"


TopViewPlot::TopViewPlot(const std::string& key_, QWidget *parent) : JustAplot(key_, parent) {
	ui = new Ui_topviewplotdlg();
	ui->setupUi(this);
	memset(ecefOrigin, 0, 3*sizeof(double));  //  this means 'not set'

	//Setup the MapGraphics scene and view
    scene = new MapGraphicsScene(this);
    view = new MapGraphicsView(scene,this);

	QVBoxLayout* VL = new QVBoxLayout(ui->viewframe);
	VL->setSpacing(1);
	VL->setContentsMargins(1, 1, 1, 1);
	VL->addWidget(view);

	 //Setup some tile sources
    QSharedPointer<OSMTileSource> osmTiles(new OSMTileSource(OSMTileSource::OSMTiles), &QObject::deleteLater);
    //QSharedPointer<OSMTileSource> aerialTiles(new OSMTileSource(OSMTileSource::MapQuestAerialTiles), &QObject::deleteLater);
    QSharedPointer<GridTileSource> gridTiles(new GridTileSource(), &QObject::deleteLater);
    QSharedPointer<CompositeTileSource> composite(new CompositeTileSource(), &QObject::deleteLater);
    composite->addSourceBottom(osmTiles);
    //composite->addSourceBottom(aerialTiles);
    composite->addSourceTop(gridTiles);
    view->setTileSource(composite);

	view->setZoomLevel(11);
	//view->centerOn(37.615149, 55.75578);
}

void TopViewPlot::setEcefOrigin(double* origin) {
	memcpy(ecefOrigin, origin, 3*sizeof(double));
}


TopViewPlot::~TopViewPlot() {
	std::list<TopViewLineObject*>::iterator it;
	for (it = lines.begin(); it != lines.end(); it++) {
		TopViewLineObject* o = *it;
		delete o;
	}


	delete ui;
}


void TopViewPlot::xlabel(const std::string&  s) {

}

void TopViewPlot::ylabel(const std::string&  s) {

}


void TopViewPlot::drawMarker(double X, double Y, int type) {

}

void TopViewPlot::addLine(LineItemInfo* line) {
	JustAplot::addLine(line);
	mxassert(fabs(ecefOrigin[0]) > 1., "");
	TopViewLineObject* o = new TopViewLineObject(line, ecefOrigin);
	lines.push_back(o);

	scene->addObject(o);
	view->centerOn(o->llaOrigin[1]*Rad2Deg, o->llaOrigin[0]*Rad2Deg);
	o->redrawRequested();

}
