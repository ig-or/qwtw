#include <QApplication>

#include <QLayout>
#include <QSlider>
#include <QLabel>

#include <QTreeView>
#include <QtCore/QDataStream>
#include <QtCore/QFile>
#include <QtCore/QIODevice>
#include <QBrush>

#define M_PI 3.1415926535897932384626433832795
#include <marble/global.h>
#include <marble/MarbleWidget.h>
#include <marble/AbstractFloatItem.h>
#include <marble/GeoDataDocument.h>
#include <marble/GeoDataPlacemark.h>
#include <marble/GeoDataLineString.h>
#include <marble/GeoDataTreeModel.h>
#include <marble/MarbleModel.h>
#include <marble/GeoPainter.h>
#include <marble/MarbleDirs.h>

#include <cstdio>

#include "xstdef.h"

using namespace Marble;

void assert_failed(const char* file, unsigned int line, const char* str) {
	xm_printf("AF: %s line %d (%s)\n", file, line, str);
}

class MyMarbleWidget: public MarbleWidget {
public:
	virtual void customPaint(GeoPainter* painter);
};
void MyMarbleWidget::customPaint(GeoPainter* painter) {
	GeoDataCoordinates home(8.4, 49.0, 0.0, GeoDataCoordinates::Degree);
	painter->setPen(Qt::green);
	painter->drawEllipse(home, 7, 7);
	painter->setPen(Qt::black);
	painter->drawText(home, "Hello Marble!");

	Marble::GeoDataCoordinates m(37.6519, 55.72218, 0.0, Marble::GeoDataCoordinates::Degree);
	QPen pen(Qt::red);
	pen.setWidth(4);
	painter->setPen(pen);

	painter->drawEllipse(m, 25, 25);


}

int main(int argc, char** argv) {
	QApplication app(argc, argv);
	MarbleDirs::setMarbleDataPath("C:/programs/marble/data");

	QWidget *window = new QWidget;

	// Create a Marble QWidget without a parent
	MarbleWidget *mapWidget = new MyMarbleWidget();

	// Load the OpenStreetMap map
	//mapWidget->setMapThemeId("earth/plain/plain.dgml");
	mapWidget->setMapThemeId("earth/openstreetmap/openstreetmap.dgml");
	//mapWidget->setMapThemeId("earth/bluemarble/bluemarble.dgml");

	GeoDataPlacemark *place = new GeoDataPlacemark("office");
	place->setCoordinate(37.6519, 55.72218, 0.0, GeoDataCoordinates::Degree);
	place->setPopulation(250);
	place->setCountryCode("Russia");
	place->setDescription("Moscow");
	//place->setStyle()

	GeoDataDocument *document = new GeoDataDocument;
	document->append(place);

	// Add the document to MarbleWidget's tree model
	mapWidget->model()->treeModel()->addDocument(document);


	mapWidget->setProjection(Mercator);

	// Enable the cloud cover and enable the country borders
	//mapWidget->setShowClouds(true);
	//mapWidget->setShowBorders(true);
	mapWidget->setShowGrid(true);
	//mapWidget->setShowTerrain(true);

	// Hide the FloatItems: Compass and StatusBar
	mapWidget->setShowOverviewMap(false);
	mapWidget->setShowScaleBar(false);

	mapWidget->setShowCompass(false);
	/*foreach(AbstractFloatItem * floatItem, mapWidget->floatItems()) {
		if (floatItem && floatItem->nameId() == "compass") {

			// Put the compass onto the left hand side
			floatItem->setPosition(QPoint(10, 10));
			// Make the content size of the compass smaller
			floatItem->setContentSize(QSize(50, 50));
		}
	}
	*/


	// Create a horizontal zoom slider and set the default zoom
	QSlider * zoomSlider = new QSlider(Qt::Horizontal);
	zoomSlider->setMinimum(1000);
	zoomSlider->setMaximum(2400);

	zoomSlider->setValue(2400);
	mapWidget->zoomView(zoomSlider->value());

	// Create a label to show the geodetic position
	QLabel * positionLabel = new QLabel();
	positionLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	// Add all widgets to the vertical layout.
	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(mapWidget);
	layout->addWidget(zoomSlider);
	layout->addWidget(positionLabel);

	// Center the map onto a given position
	GeoDataCoordinates home(37.6519, 55.72218, 0.0, GeoDataCoordinates::Degree);
	mapWidget->centerOn(home);

	// Connect the map widget to the position label.
	QObject::connect(mapWidget, SIGNAL(mouseMoveGeoPosition(QString)),
		positionLabel, SLOT(setText(QString)));

	// Connect the zoom slider to the map widget and vice versa.
	QObject::connect(zoomSlider, SIGNAL(valueChanged(int)),
		mapWidget, SLOT(zoomView(int)));
	QObject::connect(mapWidget, SIGNAL(zoomChanged(int)),
		zoomSlider, SLOT(setValue(int)));

	window->setLayout(layout);
	window->resize(400, 300);


    //mapWidget->show();
	window->show();

    return app.exec();
}
