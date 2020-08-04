
/**

	2D line view dialog.
	

	\file figure2.h
	\author   Igor Sandler
	\date    Jul 2009
	\version 1.0
	
*/ 



#ifndef FIGURE2_H
#define FIGURE2_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QToolButton>
#include <QVBoxLayout>
#include <QList>

#include "qwt_plot.h"
#include <qwt_legend.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>
#include <qwt_scale_engine.h>

#include "justaplot.h"

#include "ui_lineselect.h"

class QwtPlotRescaler;

struct FigureItem { 
	QwtPlotCurve*	line;
	LineItemInfo*	info;

	QwtPlotMarker* ma;
	FigureItem(LineItemInfo*	info_, QwtPlotCurve*	line_);
	~FigureItem();
};



class FSPlot: public QwtPlot {
public:
	FSPlot(QWidget *parent);
//private:
	QwtLegend	legend;

	virtual void resizeEvent(QResizeEvent* e);
	void setAxisSquare(bool square);
private:
	bool squareAxis;
	void doSquareAxis();

	//QwtPlotRescaler*	m_rescaler;
};


class Zoomer: public QwtPlotZoomer {
public:
    Zoomer(int xAxis, int yAxis, QWidget *canvas);
	void keepEqual(bool e);
protected:
    virtual void rescale();
private:
	bool shouldKeepAxesEqual;
};

class FSPicker : public QwtPlotPicker {
public:
	FSPicker(int xAxis, int yAxis, RubberBand rubberBand, 
		DisplayMode trackerMode, QWidget*);
	QPointF transform1(	const QPoint & 	pos	 ) 	 const;
};

class QCheckBox;
struct FigureItem;

class SLDialog : public QDialog {
	Q_OBJECT
public:
	SLDialog(std::list<FigureItem*> lines, QWidget *parent);
	std::list<FigureItem*>    getSelectedLines();
	unsigned int getWindowSize();
private:
	Ui_selectLinesDlg ui;
	std::map<QCheckBox*, FigureItem*> items;

};

class TestScaleEngine: public QwtLinearScaleEngine {
public:
    TestScaleEngine();
 
    virtual void autoScale( int maxNumSteps, double &x1, double &x2, double &stepSize ) const;
};

class Figure2 : public JustAplot {
	Q_OBJECT
public:
	Figure2(const std::string& key_, XQPlots* pf_, QWidget *parent);
	~Figure2();

	virtual void title(const std::string& s);
	virtual void footer(const std::string& s);
	virtual void xlabel(const std::string& s);
	virtual void ylabel(const std::string& s);
	virtual void drawMarker(double X, double Y, int type = 1);
	virtual void drawMarker(double t);
	virtual void makeMarkersVisible(bool visible);
	virtual void addLine(LineItemInfo* line);
	virtual void onClip(double t1, double t2);

	virtual void replot();


	void setAxesEqual();

public:

    QVBoxLayout *verticalLayout;
    QFrame *top_frame;
    QHBoxLayout *horizontalLayout;
    QToolButton *tb1;
    QToolButton *tb2;
    QToolButton *tb3;
	QToolButton *tbSaveDataToTextFile;
	QToolButton *tbFFT;
	QToolButton *tbSquareAxis;
	QToolButton* tbClip; 
	QToolButton* tbResetLayout;

	QToolButton* tbSavePicture;

    QSpacerItem *horizontalSpacer;
    FSPlot *plot1;

    void setupUi();
    void retranslateUi();

protected:
	int mode;///< figure gui mode
	bool	tbModeChanging;
	bool clipperHost;

	QwtPlotPanner *panner;
	FSPicker *picker;
	//QwtPlotZoomer *zoomer;
	Zoomer *zoomer;

	//QwtSymbol* pointMarkerSymbol;
	//QwtPlotMarker pointMarker;
	QwtPlotMarker  vLineMarker;
	std::string sTitle;
	
	std::list<FigureItem*>	lines;

	QFont titleFont, axisFont;

	void closeEvent ( QCloseEvent * event );
	void removeLines();
    void ui_addTBIcon(QToolButton* tb, const char* i);
	void setTBState();
	void onPickerSignal(int x, int y);
	void focusInEvent(QFocusEvent * event);
	std::list<FigureItem*>	selectLines();

private slots:
	void ontb1(bool checked );
	void onClip(bool checked);
	void onResetLayout();

	void ontb2(bool checked );
	void ontb3(bool checked );
	void onSaveData();
	void onSaveImage();
	void onTbFFT();
	void onTbSquareAxis(bool checked);
	void onPickerSelection(const QPolygon& pa);
	void onPickerMove(const QPoint&);

};



#endif // FIGURE2_H
