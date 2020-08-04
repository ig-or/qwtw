#pragma once



#include "xstdef.h"
#include "justaplot.h"
#include "coordtypes.h"
#include <QObject>


namespace QtDataVisualization {
	class Q3DScatter;
	class QAbstract3DGraph;
	class QAbstract3DSeries;
	class QScatter3DSeries;
//	class QScatterDataArray;
//	typedef QVector<QScatterDataItem> QScatterDataArray;
}

class Q3DView : public JustAplot {
	Q_OBJECT

public:
	//CoordType::CoordinateType ctype;
	Q3DView(const std::string& key_, XQPlots* pf_, QWidget *parent);
	virtual ~Q3DView();

	int q3Init();

	virtual void xlabel(const std::string&  s) {};
	virtual void ylabel(const std::string&  s) {};
	virtual void drawMarker(double X, double Y, int type = 1) {};
	virtual void drawMarker(double t);
	//protected:
	virtual void addLine(LineItemInfo* line);

protected slots:

	void changeStyle(int style);

	void setSmoothDots(int smooth);
	void changeTheme(int theme);

	void changePresetCamera();

	void changeLabelStyle();
	void changeFont(const QFont &font);

	void shadowQualityUpdatedByVisual(int sq);
	void selectedItemChanged(int index);

	void changeShadowQuality(int quality);
	void setBackgroundEnabled(int enabled);

	void setGridEnabled(int enabled);


signals:
	void backgroundEnabledChanged(bool);
	void gridEnabledChanged(bool);
	void fontChanged(const QFont);
	void shadowQualityChanged(int);

private:
	int m_style;
	bool m_smooth;
	double m_fontSize;
	std::map<QtDataVisualization::QScatter3DSeries*, LineItemInfo*> s;

	QtDataVisualization::Q3DScatter* scatter;
	bool selecting;
	QtDataVisualization::QScatter3DSeries* activeSeries;

	//QtDataVisualization::QScatterDataArray data;
	private slots:
	void onHome();


};
