
#include <QDialog>
#include <QMainWindow>

#ifdef USEMARBLE
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
#endif


#ifdef USEMARBLE

class MWidgetEx: public Marble::MarbleWidget {
public:

	MWidgetEx(QWidget *parent = 0);
	
protected:
	virtual void customPaint(Marble::GeoPainter* painter);
	void closeEvent(QCloseEvent * event);
private:

};

#define DEFAULT_ZOOM_LEVEL 2800
#endif


class QTTest: public QDialog {
	Q_OBJECT
public:
	QTTest(QWidget * parent1 = 0);
private:
#ifdef USEMARBLE
	MWidgetEx* mw;
#endif
};
