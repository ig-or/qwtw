
#ifndef MARBLEVIEW_H_FILE
#define MARBLEVIEW_H_FILE


#include "xstdef.h"
#include "justaplot.h"
#include "coordtypes.h"
//#include "marble/GeoDataLineString.h"


class MWidgetEx;
class ENUframe;

/**
@return 0 if all is OK
*/
int setMarbleDataPath(const char* p);

class MarView: public JustAplot {
	Q_OBJECT

public:
	CoordType::CoordinateType ctype;
	MarView(const std::string& key_, XQPlots* pf_, QWidget *parent);
	virtual ~MarView();

	int mvInit();

	virtual void xlabel(const std::string&  s) {};
	virtual void ylabel(const std::string&  s) {};
	virtual void drawMarker(double X, double Y, int type = 1) {};
	virtual void drawMarker(double t);
//protected:
	virtual void addLine(LineItemInfo* line);

private:
	MWidgetEx* mw;
	ENUframe* fa;

private slots:
	void onHome();

};

#endif