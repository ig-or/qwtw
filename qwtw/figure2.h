
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
#include <QLineEdit>

#include "qwt_plot.h"
#include <qwt_legend.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
//
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>
#include <qwt_scale_engine.h>

#include "justaplot.h"

#include "ui_lineselect.h"

class QwtPlotCurve;
class QwtPlotMarker;
class QwtPlotRescaler;


struct FigureItem {
	QwtPlotCurve* line;
	LineItemInfo* info;
	//long long key;

	QwtPlotMarker* ma;
	FigureItem(LineItemInfo* info_, QwtPlotCurve* line_);
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
class SelectInfoDlg : public QDialog {
	Q_OBJECT
public:
	SelectInfoDlg(QWidget *parent);
	QLineEdit* text;
	QVBoxLayout* verticalLayout;
	bool ret;
protected:
	bool eventFilter(QObject *obj, QEvent *event) override;
	void keyPressEvent( QKeyEvent *k );
};

class SelectNameDlg : public SelectInfoDlg {
	Q_OBJECT
public: 
	SelectNameDlg(QWidget *parent = 0, const char* name = 0);
};

class SelectMarkerParamsDlg : public SelectInfoDlg {
	Q_OBJECT
public:
	SelectMarkerParamsDlg(QWidget *parent = 0, const char* name = 0);
	QPushButton* cpb;
	QPushButton* okpb;
	QColor selectedColor;

public slots:
	void onColor();
	void onOK();

};

class TestScaleEngine: public QwtLinearScaleEngine {
public:
	TestScaleEngine();
 
	virtual void autoScale( int maxNumSteps, double &x1, double &x2, double &stepSize ) const;
};

class QWMarker : public QwtPlotMarker {
public:
	QWMarker(int id_) : id(id_) {}
	int id;
};

class VLineMarker :public QWMarker {
	public:
	double t;
	VLineMarker(const char* text, double time, int id_ = 0);
};

class AMarker : public QWMarker  {
	public:
	double x, y;
	QColor color;
	AMarker(const char* text, double x_, double y_, const QColor& color_, int id_ = 0);
};


class ArrowSymbol: public QwtSymbol  {
public:
	ArrowSymbol();
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
	virtual void removeLine(LineItemInfo* line);
	virtual void changeLine(LineItemInfo* line, double* x, double* y, double* z, double* time, int size);
	void addVMarker();
	void addGlobalVMarker();
	/**  test what we can do about the marker.
	 * 	\param type 1 - VMarker, 2 - AMarker
	 *  \param mid marker ID to remove
	 * \return 	0 do nothing
	 * 			1 add a marker
	 * 			2 remove a marker
	 */ 
	int markerTest(int type, int& mid, std::string& label, QColor& color);
	virtual void addVMarker(double t, const char* label = 0, int id = 0);
	virtual void removeVMarker(int id_);
	void addAMarker();


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
	void keyPressEvent( QKeyEvent *k );

	std::list<FigureItem*>	selectLines();
	std::list<VLineMarker*> vmList;
	std::list<AMarker*> amList;
private:
	double lastXselected, lastYselected;
	bool pointWasSelected;

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
