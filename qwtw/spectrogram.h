#pragma once



#include "justaplot.h"
#include "qwtypes.h"
#include <qwt_plot.h>
#include <string>

#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QToolButton>
#include <QVBoxLayout>

class QwtPlotSpectrogram;
class QwtPlotZoomer;
class QwtPlotPicker;
class QwtPlotPanner;
class MyZoomer;
class FSPicker2;

class QSpectrogram : public QwtPlot {
    Q_OBJECT

public:
    enum ColorMap    {
        RGBMap,
        HueMap
    };

    QSpectrogram(QWidget* = NULL, unsigned int flags_ = 0);
	virtual void resizeEvent(QResizeEvent* e);

Q_SIGNALS:
    void rendered(const QString& status);

public Q_SLOTS:
    void showContour(bool on);
    void showSpectrogram(bool on);

    void setColorMap(int);
    void setColorTableSize(int);
    void setAlpha(int);
	void setAxesEqual();
	
	void setAxisSquare(bool square);
	void setInfo(const SpectrogramInfo& info);
	void onPickerSelection(const QPolygon&);
	void onPickerMove(const QPoint&);
	void enablePicker(bool);
private:
	bool	squareAxis = false;
    virtual void drawItems(QPainter*, const QRectF&,
        const QwtScaleMap maps[axisCnt]) const QWT_OVERRIDE;

	void doSquareAxis();

    QwtPlotSpectrogram* d_spectrogram;
	MyZoomer* zoomer;
	FSPicker2* picker;

    int d_mapType;
    int d_alpha;
};

class QSpectrogramPlot : public JustAplot {
	Q_OBJECT
public:
	QSpectrogramPlot(const std::string& key_, XQPlots* pf_, QWidget* parent, unsigned int flags_ = 0);
	~QSpectrogramPlot();

	virtual void title(const std::string& s);
	virtual void footer(const std::string& s);
	virtual void xlabel(const std::string& s);
	virtual void ylabel(const std::string& s);
	virtual void drawMarker(double X, double Y, int type = 1);
	virtual void drawMarker(double t);
	virtual void makeMarkersVisible(bool visible);
	virtual void onClip(double t1, double t2);
	virtual void replot();
	//void setAxesEqual();
	void setInfo(const SpectrogramInfo& info);

	QVBoxLayout* verticalLayout;
	//QFrame* top_frame;
	//QHBoxLayout* horizontalLayout;
	QToolButton* tbPicker;
	//QToolButton* tb2;
	//QToolButton* tb3;
	//QToolButton* tbSaveDataToTextFile;
	QToolButton* tbSquareAxis;
	QToolButton* tbClip;
	QToolButton* tbResetLayout;

	QToolButton* tbSavePicture;

	QSpacerItem* horizontalSpacer;
	QSpectrogram* spectrogram = nullptr;

	void setupUi();
	void retranslateUi();

protected:
	int mouseMode;///< figure gui mode
	unsigned int flags;
	bool	tbModeChanging;
	bool	clipperHost;
	
	//QwtPlotPanner* panner;

	std::string sTitle;

	QFont titleFont, axisFont;

	void closeEvent(QCloseEvent* event);
	void removeLines();
	void ui_addTBIcon(QToolButton* tb, const char* i);
	void setTBState();
	void onPickerSignal(int x, int y);
	void focusInEvent(QFocusEvent* event);
	void keyPressEvent(QKeyEvent* k);
	
	

	
private:
	double lastXselected, lastYselected;
	bool pointWasSelected;

private slots:
	void ontbPicker(bool checked);
	void onClip(bool checked);
	void onResetLayout();

	void ontb2(bool checked);
	void ontb3(bool checked);
	void onSaveData();
	void onSaveImage();
	void onTbFFT();
	void onTbSquareAxis(bool checked);
	//void onPickerSelection(const QPolygon& pa);
	//void onPickerMove(const QPoint&);
};

