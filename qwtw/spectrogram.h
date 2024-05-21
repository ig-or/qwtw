/** everything about the spectrogramm.
*/

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
class SpectrogramData;
class QwtPlotMarker;
class QSpectrogramPlot;
class FSPicker2;
class AMarker;


class QSpectrogram : public QwtPlot {
    Q_OBJECT

public:
	enum ColorMap
	{
		RGBMap,
		HueMap,
		SaturationMap,
		ValueMap,
		SVMap,
		AlphaMap
	};
	double lastXselected, lastYselected;
    QSpectrogram(QWidget* = NULL, unsigned int flags_ = 0);
	virtual ~QSpectrogram();
	virtual void resizeEvent(QResizeEvent* e);
	bool haveTimeInfo() const;
	bool havePointsInfo() const;
	void spClip(double x1, double x2, double y1, double y2);

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
	void drawMarker(double t);
	void drawMarker3D(double* x);
private:
	bool	squareAxis = false;
	
    virtual void drawItems(QPainter*, const QRectF&,
        const QwtScaleMap maps[axisCnt]) const QWT_OVERRIDE;

	void doSquareAxis();
	void onPickerSignal(int x, int y);

    QwtPlotSpectrogram* d_spectrogram = nullptr;
	QSpectrogramPlot* spp;
	SpectrogramData* sData = nullptr;
	MyZoomer* zoomer = nullptr;
	FSPicker2* picker = nullptr;
	QwtPlotMarker* ma = nullptr;
	bool maIsVisible = false;
	std::string legend;

    int d_mapType;
    int d_alpha;
};


/**
This is a dialog window with a spectrogramm.
*/
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
	virtual void draw3DMarker(const CBPickerInfo& cpi);
	virtual void makeMarkersVisible(bool visible);
	virtual void onClip(double t1, double t2);
	void spClip(double x1, double x2, double y1, double y2);
	virtual int savePng(const std::string& filename);
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

	void picker_t(double t);
	void picker_p(double* p);
	bool haveTimeInfo() const;
	bool havePointsInfo() const;


protected:
	int mouseMode;///< figure gui mode
	unsigned int flags;
	bool	tbModeChanging;
	bool	clipperHost;
	
	//QwtPlotPanner* panner;

	std::string sTitle;

	QFont titleFont, axisFont;
	std::list<AMarker*> amList;

	void closeEvent(QCloseEvent* event);
	void removeLines();
	void ui_addTBIcon(QToolButton* tb, const char* i);
	void setTBState();
	void onPickerSignal(int x, int y);
	void focusInEvent(QFocusEvent* event);
	void keyPressEvent(QKeyEvent* k);
	
private:
	//double lastXselected, lastYselected;
	bool pointWasSelected;
	std::string legend;

	/**  test what we can do about the marker.
 *  \param mid marker ID to remove
 * \return 	0 do nothing
 * 			1 add a marker
 * 			2 remove a marker
 */
	int markerTest(int& mid, std::string& label, QColor& color);
	void addAMarker();

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

