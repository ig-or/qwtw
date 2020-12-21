

#define M_PI 3.14159265358979323846264338327950288419717

#include <marble/GeoDataLineString.h>

#include <marble/MarbleWidget.h>
#include <marble/AbstractFloatItem.h>
#include <marble/GeoDataDocument.h>
#include <marble/GeoDataPlacemark.h>

#include <marble/GeoDataTreeModel.h>
#include <marble/MarbleModel.h>
#include <marble/GeoPainter.h>
#include <marble/MarbleDirs.h>
#include <marble/GeoDataLatLonBox.h>

#include <marble/LayerInterface.h>

#include "xstdef.h"
#include "sfigure.h"
#include "xmcoords.h"
#include "xmutils.h"

#include <QLayout>
#include <QLabel>
#include <QtCore/QDataStream>
#include <QTreeView>

#include <QtCore/QFile>
#include <QtCore/QIODevice>
#include <QBrush> 
#include <QToolButton>

#include <QDebug>
//#include <QtNetwork/qssl.h>
//#include <QtNetwork/qsslsocket.h>

//#include <marble/global.h>


#include "marbleview.h"

#define DEFAULT_ZOOM_LEVEL 2800

using namespace Marble;

static bool mpWasSet = false;
/*
int setMarbleDataPath(const char* p) {
	MarbleDirs::setMarbleDataPath(p);
	return 0;
}
*/
class MWPaintLayer: public LayerInterface {

};

class MWidgetEx: public MarbleWidget {
public:

	MWidgetEx(QWidget *parent = 0);
	
	void addLine(LineItemInfo* line);
	void drawMarker(double t);
	void onHome();
protected:
	virtual void customPaint(GeoPainter* painter);
	void closeEvent(QCloseEvent * event);
private:
	struct Marker {
		bool active;
		Marble::GeoDataCoordinates coord;
		Marker(): active(false) {}
	};
	struct Line {
		LineItemInfo* info;
		Marble::GeoDataLineString* geo;

		std::vector<Marble::GeoDataCoordinates> geoCoordsBackup;

		double maxLat, maxLon, minLat, minLon;
		Marker ma;
	};
	double maxLat, maxLon, minLat, minLon;
	std::list<Line> lines;
	void removeLines();
};

MWidgetEx::MWidgetEx(QWidget *parent): MarbleWidget(parent) {
	maxLat = 0; maxLon = 0; minLat = 0; minLon = 0;
	/*
	bool haveSSL = QSslSocket::supportsSsl();

	if (!haveSSL) {
		xm_printf("ERROR: looks like SSL not working; no maps will be downloaded\n");
		xm_printf("MWidgetEx ssl info: build version = %s, supports = %s,  lib version =  %s\n",
			QSslSocket::sslLibraryBuildVersionString().toUtf8().constData(),
			haveSSL ? "yes" : "no",
			QSslSocket::sslLibraryVersionString().toUtf8().constData());

	}
*/
}

void MWidgetEx::addLine(LineItemInfo* line) {
	Marble::GeoDataLineString* s = new Marble::GeoDataLineString();
	s->setTessellationFlags(Marble::NoTessellation);
	s->clear();
	int i;
	Line f;
	f.maxLat = line->y[0];
	f.maxLon = line->x[0];
	f.minLat = line->y[0];
	f.minLon = line->x[0];
	//f.geoCoordsBackup.resize(line->size);
	for (i = 0; i < line->size; i++) {
		s->append(Marble::GeoDataCoordinates(line->x[i], line->y[i], 0., Marble::GeoDataCoordinates::Degree));
		if (f.maxLat < line->y[i]) f.maxLat = line->y[i];
		if (f.minLat > line->y[i]) f.minLat = line->y[i];
		if (f.maxLon < line->x[i]) f.maxLon = line->x[i];
		if (f.minLon > line->x[i]) f.minLon = line->x[i];
		//f.geoCoordsBackup[i] = Marble::GeoDataCoordinates(line->x[i], line->y[i], 0., Marble::GeoDataCoordinates::Degree);
	}
	f.info = line; f.geo = s;

	if (lines.size() == 0) { //   the very first line
		maxLat = f.maxLat ;
		minLat = f.minLat ;
		maxLon = f.maxLon ;
		minLon = f.minLon ;
	} else { // next line
		if (maxLat < f.maxLat) maxLat = f.maxLat;
		if (minLat > f.minLat) minLat = f.minLat;
		if (maxLon < f.maxLon) maxLon = f.maxLon;
		if (minLon > f.minLon) minLon = f.minLon;
	}
	lines.push_back(f);

	GeoDataLatLonBox box(maxLat, minLat, maxLon, minLon, Marble::GeoDataCoordinates::Unit::Degree);
	centerOn(box, false);
	//xm_printf("center on: %f %f %f  %f\n ", maxLat, minLat, maxLon, minLon);
	update();
}

void MWidgetEx::onHome() {
	GeoDataLatLonBox box(maxLat, minLat, maxLon, minLon, Marble::GeoDataCoordinates::Unit::Degree);
	centerOn(box, false);


	update();
}

void MWidgetEx::removeLines() {
	if (lines.size() == 0) {
		return;
	}
	for (std::list<Line>::iterator i = lines.begin(); i != lines.end(); i++) {
		Line s = *i;
		s.geo->clear();
		delete s.geo;
		s.geo = 0;
		s.geoCoordsBackup.clear();
		std::vector<Marble::GeoDataCoordinates>().swap(s.geoCoordsBackup);
	}
	lines.clear();
}
void MWidgetEx::closeEvent(QCloseEvent * event) {
	removeLines();
}

void MWidgetEx::customPaint(GeoPainter* painter) {
	//return;

	Marble::GeoDataCoordinates m(37.6519, 55.72218, 0.0, Marble::GeoDataCoordinates::Degree);
	QPen pen(Qt::green);
	pen.setWidth(4);
	painter->setPen(pen);
	painter->drawEllipse(m, 25, 25);
	int count = 0;
	QColor color = Qt::darkBlue;
	if (lines.size() != 0) {
		for (std::list<Line>::iterator i = lines.begin(); i != lines.end(); i++) {
			pen.setColor(color);
			pen.setWidth(i->info->lineWidth);
			char symStyle = 0;
			Qt::PenStyle lineStyle = Qt::SolidLine;
			if (i->info->style == std::string()) {
				//
			} else {
				int sn = i->info->style.size();
				if (sn > 0) { //    last is always color:
					//  set color:
					switch (i->info->style[sn - 1]) {
						case 'r': color = Qt::red; break;
						case 'd': color = Qt::darkRed;  break;
						case 'k': color = Qt::black;  break;
						case 'w': color = Qt::white; break;
						case 'g': color = Qt::green;  break;
						case 'G': color = Qt::darkGreen;  break;
						case 'm': color = Qt::magenta;  break;
						case 'M': color = Qt::darkMagenta;  break;
						case 'y': color = Qt::yellow;  break;
						case 'Y': color = Qt::darkYellow;  break;
						case 'b': color = Qt::blue;  break;
						case 'c': color = Qt::cyan;  break;
						case 'C': color = Qt::darkCyan;  break;
					};
					pen.setColor(color);
				}
				if (sn > 1) {  //  first is always a line style:
					switch (i->info->style[0]) {
						case ' ': lineStyle = Qt::NoPen;  break;
						case '-': lineStyle = Qt::SolidLine;  break;
						case '%': lineStyle = Qt::DashLine;  break;
						case '#': lineStyle = Qt::DashDotDotLine;  break;
						case '.': lineStyle = Qt::DotLine;  break;
					};
					pen.setStyle(lineStyle);
				}
				if (sn == 3) {  //   middle is symbol type
					symStyle = i->info->style[1];
				}
			}

			painter->setPen(pen);
			//GeoDataLineString* s = i->geo;
			if (lineStyle != Qt::NoPen) {
				painter->drawPolyline(*(i->geo));
			} else {
				pen.setStyle(Qt::SolidLine);
				painter->setPen(pen);
			}
			if (symStyle != 0) {
				//painter->drawPoints()
				int symSize = i->info->symSize;
				pen.setStyle(Qt::SolidLine);

				int ls = i->geo->size();

				//QVector<GeoDataCoordinates>::Iterator it;
				QVector<GeoDataCoordinates>::ConstIterator cit;
				
				/*
				for (cit = i->geo->constBegin(); cit != i->geo->constEnd(); cit++) {
				//std::vector<Marble::GeoDataCoordinates>::iterator it;
				//for (it = i->geoCoordsBackup.begin(); it != i->geoCoordsBackup.end(); it++) {
					switch (symStyle) {
						case 'e':	painter->drawEllipse(*cit, symSize, symSize);  break;
						case 'r':	painter->drawRect(*cit, symSize, symSize);  break;
						default:  painter->drawEllipse(*cit, symSize, symSize);  break;

					};
				}
				*/
				for (int ii = 0; ii < ls; ii++) {
					switch (symStyle) {
					case 'e':	painter->drawEllipse(i->geo->at(ii), symSize, symSize);  break;
					case 'r':	painter->drawRect(i->geo->at(ii), symSize, symSize);  break;
					default:  painter->drawEllipse(i->geo->at(ii), symSize, symSize);  break;

					};
				}

				
				
			}

			// draw legend:
			QFont fo = painter->font();
			fo.setPointSize(11);
			painter->setFont(fo);
			painter->drawText(8, 20 + 20 * count, i->info->legend.c_str());

			//  draw marker:
			//pen.setColor(Qt::black);
			pen.setWidth(3);
			pen.setBrush(color);
			pen.setColor(Qt::black);
			painter->setPen(pen);
			if (i->ma.active) {
				painter->drawRect(i->ma.coord, 7, 7); 
			}


			count++;
		}
		
	}
}

void MWidgetEx::drawMarker(double t) {
	

	if (lines.size() == 0) {
		return;
	}
	for (std::list<Line>::iterator i = lines.begin(); i != lines.end(); i++) {
		if (i->info->time == 0) {
			continue;
		}
		i->ma.active = true;
		long long index = 0;
		//xm_printf("findClosestPoint_1 failed; siae = %u; time = %u, t = %f\n",
		//	i->info->size, i->info->time, t);
		mxat(i->info->size > 0);
		index = findClosestPoint_1(0, i->info->size - 1, i->info->time, t);

		i->ma.coord = (*i->geo)[index];
	}
	 
	update();
}

MarView::MarView(const std::string& key_, XQPlots* pf_, QWidget *parent) : JustAplot(key_, pf_, parent, 2) {
	if (!mpWasSet) {
		mpWasSet = true;
//#ifdef VCPKG_MARBLE_DATA_PATH
		//xm_printf("Trying to use marble data path1 %s \n ", XQX9STR(VCPKG_MARBLE_DATA_PATH));
		//Marble::MarbleDirs::setMarbleDataPath(XQX9STR(VCPKG_MARBLE_DATA_PATH));
//#else
/*
	#ifdef WIN32
			std::string cp = getCommonAppDataPath();
			cp.append("\\qwtw\\marble-data");
			xm_printf("Trying to use marble data path2 %s\n", cp.c_str());
			setMarbleDataPath(cp.c_str());
	#else
			setMarbleDataPath("/usr/share/marble/data");
	#endif
	*/
//#endif
	}
	mw = 0;
	ctype = CoordType::ECEF;
	fa = 0;
}

MarView::~MarView() {
	if (mw != 0) {

		mw = 0; // ?
	}
	if (fa != 0) {
		delete fa; fa = 0;
	}
	emit exiting(key);
}

int MarView::mvInit() {
	if (mw != 0) return 1;
	mw = new MWidgetEx(this);
	// Load the OpenStreetMap map
	//mw->setMapThemeId("earth/plain/plain.dgml");
	mw->setMapThemeId("earth/openstreetmap/openstreetmap.dgml");
	//mw->setMapThemeId("earth/bluemarble/bluemarble.dgml");

	/*
	GeoDataPlacemark *place = new GeoDataPlacemark("office");
	place->setCoordinate(37.6519, 55.72218, 0.0, GeoDataCoordinates::Degree);
	place->setPopulation(250);
	place->setCountryCode("Russia");
	place->setDescription("Moscow");
	*/
	GeoDataDocument *document = new GeoDataDocument;
	//document->append(place);

	// Add the document to MarbleWidget's tree model
	mw->model()->treeModel()->addDocument(document);

	mw->setProjection(Mercator);

	// Enable the cloud cover and enable the country borders
	//mapWidget->setShowClouds(true);
	mw->setShowBorders(true);
	mw->setShowGrid(true);
	mw->showGrid();
	//mw->setAnimationsEnabled(true);
	//mw->setViewContext(Marble::Animation);
	mw->setShowCities(true);
	//mw->setShowTerrain(true);

	// Hide the FloatItems: Compass and StatusBar
	mw->setShowOverviewMap(false);
	//mw->setShowScaleBar(false);

	//mw->setShowCompass(false);
	/*foreach(AbstractFloatItem * floatItem, mapWidget->floatItems()) {
	if (floatItem && floatItem->nameId() == "compass") {

	// Put the compass onto the left hand side
	floatItem->setPosition(QPoint(10, 10));
	// Make the content size of the compass smaller
	floatItem->setContentSize(QSize(50, 50));
	}
	}
	*/

	mw->zoomView(DEFAULT_ZOOM_LEVEL);

	// Create a label to show the geodetic position
	QLabel * positionLabel = new QLabel();
	positionLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	//   top frame:
	QFrame* top_frame = new QFrame(this);
	top_frame->setObjectName(QString::fromUtf8("top_frame"));
	top_frame->setMinimumSize(QSize(0, 32));
	top_frame->setMaximumHeight(32);
	top_frame->setFrameShape(QFrame::NoFrame);
	top_frame->setFrameShadow(QFrame::Raised);
	top_frame->setLineWidth(1);
	QHBoxLayout* horizontalLayout = new QHBoxLayout(top_frame);
	horizontalLayout->setSpacing(2);
	horizontalLayout->setMargin(2);

	QToolButton* tbHome = new QToolButton(top_frame);
	tbHome->setIcon(QIcon(QPixmap(":/icons/binokl.png")));
/*
	QIcon icon;
	QPixmap pm(":/icons/binokl.png");

	QImage sourceImage;
	sourceImage.load(":/icons/binokl.png");
		
	//icon.addPixmap(pm, QIcon::Normal, QIcon::Off);
	tb1->setIcon(icon);
	tb1->setIcon(QPixmap::fromImage(sourceImage));
	*/
	tbHome->setText("go home");
	tbHome->setToolTip("go home");
	horizontalLayout->addWidget(tbHome);
	connect(tbHome, SIGNAL(clicked()), this, SLOT(onHome()));


	QSpacerItem* horizontalSpacer = new QSpacerItem(244, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	horizontalLayout->addItem(horizontalSpacer);

	// Add all widgets to the vertical layout.
	QVBoxLayout *layout = new QVBoxLayout;

	layout->setSpacing(2); layout->setMargin(2);
	layout->addWidget(top_frame);
	layout->addWidget(mw);
	layout->addWidget(positionLabel);

	// Center the map onto a given position
	GeoDataCoordinates home(37.6519, 55.72218, 0.0, GeoDataCoordinates::Degree);
	mw->centerOn(home);

	// Connect the map widget to the position label.
	QObject::connect(mw, SIGNAL(mouseMoveGeoPosition(QString)),
		positionLabel, SLOT(setText(QString)));

	setLayout(layout);
	resize(400, 300);

	show();
	return 0;
}

void MarView::onHome() {
	mw->onHome();
}

void MarView::addLine(LineItemInfo* line) {
	JustAplot::addLine(line);
	//double m = fabs(line->x)

	mw->addLine(line);
}
void MarView::drawMarker(double t) {
	JustAplot::drawMarker(t);
	mw->drawMarker(t);
}
