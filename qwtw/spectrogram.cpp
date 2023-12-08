
#include "spectrogram.h"

#include <qwt_color_map.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_renderer.h>
#include <qwt_picker_machine.h>
#include <qwt_interval.h>
#include <qpen.h>
#include <qtoolbar.h>
#include <qstatusbar.h>
#include <qelapsedtimer.h>
#include <qcheckbox.h>
#include "settings.h"

#include <limits>
#include <cmath>

int xmprintf(int level, const char* _Format, ...);

class MyZoomer : public QwtPlotZoomer  {
public:
    MyZoomer(QWidget* canvas, QwtRasterData* rd_) :
        QwtPlotZoomer(canvas), rd(rd_)
    {
        setTrackerMode(AlwaysOn);
    }

    virtual QwtText trackerTextF(const QPointF& pos) const QWT_OVERRIDE    {
        QColor bg(Qt::white);
        bg.setAlpha(185);
        double x = pos.x();
        double y = pos.y();
        double z = rd->value(x, y);
        char stmp[256];
        snprintf(stmp, 256, "%.4f (%.4f, %4f)", z, x, y);
        stmp[255] = 0;
        QwtText text = QString::fromUtf8(stmp);
        text.setBackgroundBrush(QBrush(bg));
        QFont font("Helvetica [Cronyx]", 12);
        text.setFont(font);
        return text;
    }
    void keepAxesEqual(bool e) {
        shouldKeepAxesEqual = e;
    }
    void replaceRasterData(QwtRasterData* rd_) {
        rd = rd_;
    }
    virtual QRect trackerRect(const QFont& f) const QWT_OVERRIDE {
        QRect rect = QwtPicker::trackerRect(f);

        return rect;
    }
private:
    bool shouldKeepAxesEqual = false;
    QwtRasterData* rd = nullptr;
};

class FSPicker2 : public QwtPlotPicker {
public:
    FSPicker2(int xAxis, int yAxis, RubberBand rubberBand,
        DisplayMode trackerMode, QWidget*);
    QPointF transform1(const QPoint& pos) 	 const;
    void setRasterData(QwtRasterData* rd_) {
        rd = rd_;
    }

protected:
    QwtRasterData* rd = nullptr;

    virtual QwtText FSPicker2::trackerTextF(const QPointF& pos) const;

};


FSPicker2::FSPicker2(int xAxis, int yAxis, RubberBand rubberBand,
    DisplayMode trackerMode, QWidget* canv) :
    QwtPlotPicker(xAxis, yAxis, rubberBand, trackerMode, canv) {

    /*  d_picker = new QwtPlotPicker( QwtPlot::xBottom, QwtPlot::yLeft,
    QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn,
    d_plot->canvas() );
d_picker->setStateMachine( new QwtPickerDragPointMachine() );
d_picker->setRubberBandPen( QColor( Qt::green ) );
d_picker->setRubberBand( QwtPicker::CrossRubberBand );
d_picker->setTrackerPen( QColor( Qt::white ) );

    */

}

QPointF FSPicker2::transform1(const QPoint& pos) 	 const {
    QPointF ret = invTransform(pos);
    return ret;
}

QwtText FSPicker2::trackerTextF(const QPointF& pos) const {
    QColor bg(Qt::white);
    bg.setAlpha(185);
    double x = pos.x();
    double y = pos.y();
    double z = 0.0;
    if (rd != nullptr) {
        z = rd->value(x, y);
    }
    char stmp[256];
    snprintf(stmp, 256, "%.4f (%.4f, %4f)", z, x, y);
    stmp[255] = 0;
    QwtText text = QString::fromUtf8(stmp);
    text.setBackgroundBrush(QBrush(bg));
    QFont font("Helvetica [Cronyx]", 12);
    text.setFont(font);
    return text;
}


class LinearColorMap : public QwtLinearColorMap
{
public:
    LinearColorMap(int formatType) :
        QwtLinearColorMap(Qt::darkCyan, Qt::red)
    {
        setFormat((QwtColorMap::Format)formatType);

        addColorStop(0.1, Qt::cyan);
        addColorStop(0.6, Qt::green);
        addColorStop(0.95, Qt::yellow);
    }
};

class HueColorMap : public QwtHueColorMap
{
public:
    HueColorMap(int formatType) :
        QwtHueColorMap(QwtColorMap::Indexed)
    {
        setFormat((QwtColorMap::Format)formatType);

        //setHueInterval( 240, 60 );
        //setHueInterval( 240, 420 );
        setHueInterval(0, 359);
        setSaturation(150);
        setValue(200);
    }
};



class TestSpectrogramData : public QwtRasterData
{
public:
    TestSpectrogramData()
    {
        // some minor performance improvements when the spectrogram item
        // does not need to check for NaN values

        setAttribute(QwtRasterData::WithoutGaps, true);

        d_intervals[Qt::XAxis] = QwtInterval(-1.5, 1.5);
        d_intervals[Qt::YAxis] = QwtInterval(-1.5, 1.5);
        d_intervals[Qt::ZAxis] = QwtInterval(0.0, 10.0);
    }

    virtual QwtInterval interval(Qt::Axis axis) const QWT_OVERRIDE
    {
        if (axis >= 0 && axis <= 2)
            return d_intervals[axis];

        return QwtInterval();
    }

    virtual double value(double x, double y) const QWT_OVERRIDE
    {
        const double c = 0.842;
        //const double c = 0.33;

        const double v1 = x * x + (y - c) * (y + c);
        const double v2 = x * (y + c) + x * (y + c);

        return 1.0 / (v1 * v1 + v2 * v2);
    }

private:
    QwtInterval d_intervals[3];
};


class SpectrogramData : public QwtRasterData
{
public:
    SpectrogramData(const SpectrogramInfo& info)
    {
        // some minor performance improvements when the spectrogram item
        // does not need to check for NaN values
        setAttribute(QwtRasterData::WithoutGaps, true);
        
        if ((info.nx < 2) || (info.ny < 2) || (info.xmin > info.xmax) || (info.ymin > info.ymax) || (info.z == 0)) {
            xmprintf(1, "SpectrogramData: ERROR in parameters \n");
            return;
        }

        //  copy the data:
        xSize = info.nx;
        ySize = info.ny;
        int nz = info.nx * info.ny;
        try {
            //yData = new double[info.ny];
            zData = new double[nz];
            //xData = new double[info.nx];
        } catch (std::exception& ex) {
            xmprintf(1, "SpectrogramData: ERROR in memory allocation (%s) \n", ex.what());
            return;
        }
        //memcpy(xData, info.x, xSize * sizeof(double));
        //memcpy(yData, info.y, ySize * sizeof(double));
        memcpy(zData, info.z, nz * sizeof(double));

        //  save intervals
        xMin = info.xmin; xMax = info.xmax;
        yMin = info.ymin; yMax = info.ymax;
        d_intervals[Qt::XAxis] = QwtInterval(xMin, xMax);
        d_intervals[Qt::YAxis] = QwtInterval(yMin, yMax);

        zMin = std::numeric_limits<double>::max();
        zMax = std::numeric_limits<double>::min();
        for (int i = 0; i < nz; i++) {
            if (zMin > zData[i]) zMin = zData[i];
            if (zMax < zData[i]) zMax = zData[i];
        }
        d_intervals[Qt::ZAxis] = QwtInterval(zMin, zMax);
        wx = xMax - xMin;
        wy = yMax - yMin;

        dx = wx / (info.nx - 1.0);
        dy = wy / (info.ny - 1.0);
    }

    virtual QwtInterval interval(Qt::Axis axis) const QWT_OVERRIDE
    {
        if (axis >= 0 && axis <= 2)
            return d_intervals[axis];

        return QwtInterval();
    }

    virtual double value(double x, double y) const QWT_OVERRIDE
    {
        if ((zData == 0) || (xSize == 0)) {
            return 0;
        }
        int ix = 0;
        int iy = 0;
        if (x >= xMax) {
            ix = xSize - 1;
        } else if (x <= xMin) {
            ix = 0;
        } else {
            ix = std::lround((x - xMin) / dx);
        }
        if (y >= yMax) {
            iy = ySize - 1;
        } else if (y <= yMin) {
            iy = 0;
        } else {
            iy = std::lround((y - yMin) / dy);
        }

        if (ix < 0) ix = 0;
        if (ix >= xSize) ix = xSize - 1;
        if (iy < 0) iy = 0;
        if (iy >= ySize) iy = ySize - 1;
        return zData[iy * xSize + ix];

    }
    void clearData() {
        /*
        if (xData != 0) {
            delete[] xData; xData = 0;
        }
        if (yData != 0) {
            delete[] yData; yData = 0;
        }
        */
        if (zData != 0) {
            delete[] zData; zData = nullptr;
        }
        xSize = 0;
        ySize = 0;
    }

    ~SpectrogramData() {
        clearData();
    }
private:
    QwtInterval d_intervals[3];
    int xSize = 0;          /// number of horizontal points
    int ySize = 0;          /// number of vertical points
    //double* xData = 0;      // hor points 
    //double* yData = 0;      // ver points
    double* zData = nullptr;      // the[ xSize x ySize] matrix of all the points
    double dx, dy;
    double zMin, zMax, xMin, xMax, yMin, yMax, wx, wy;
};



class Spectrogram : public QwtPlotSpectrogram
{
public:
    int elapsed() const     {
        return d_elapsed;
    }

    QSize renderedSize() const     {
        return d_renderedSize;
    }

protected:
    virtual QImage renderImage(
        const QwtScaleMap& xMap, const QwtScaleMap& yMap,
        const QRectF& area, const QSize& imageSize) const QWT_OVERRIDE
    {
#if QT_VERSION < 0x040700
        QTime t;
#else
        QElapsedTimer t;
#endif
        t.start();

        QImage image = QwtPlotSpectrogram::renderImage(
            xMap, yMap, area, imageSize);

        d_elapsed = t.elapsed();
        d_renderedSize = imageSize;

        return image;
    }

private:
    mutable int d_elapsed = 0;
    mutable QSize d_renderedSize;
};


QSpectrogram::QSpectrogram(QWidget* parent, unsigned int flags_) :
        QwtPlot(parent),
        d_alpha(255)   {
    d_spectrogram = new Spectrogram();
    d_spectrogram->setRenderThreadCount(0); // use system specific thread count
    d_spectrogram->setCachePolicy(QwtPlotRasterItem::PaintCache);

    QList<double> contourLevels;
    for (double level = 0.5; level < 10.0; level += 1.0)
        contourLevels += level;
    d_spectrogram->setContourLevels(contourLevels);
    QwtRasterData* data = new TestSpectrogramData();
    d_spectrogram->setData(data);
    d_spectrogram->attach(this);

    const QwtInterval zInterval = d_spectrogram->data()->interval(Qt::ZAxis);

    // A color bar on the right axis
    QwtScaleWidget* rightAxis = axisWidget(QwtPlot::yRight);
    rightAxis->setTitle("Intensity");
    rightAxis->setColorBarEnabled(true);

    setAxisScale(QwtPlot::yRight, zInterval.minValue(), zInterval.maxValue());
    enableAxis(QwtPlot::yRight);

    plotLayout()->setAlignCanvasToScales(true);

    setColorMap(QSpectrogram::RGBMap);

    // LeftButton for the zooming
    // MidButton for the panning
    // RightButton: zoom out by 1
    // Ctrl+RighButton: zoom out to full size

    zoomer = new MyZoomer(canvas(), data);
    zoomer->setMousePattern(QwtEventPattern::MouseSelect2,
        Qt::RightButton, Qt::ControlModifier);
    zoomer->setMousePattern(QwtEventPattern::MouseSelect3,
        Qt::RightButton);

    QwtPlotPanner* panner = new QwtPlotPanner(canvas());
    panner->setAxisEnabled(QwtPlot::yRight, false);
    panner->setMouseButton(Qt::MidButton);

    picker = new FSPicker2(QwtPlot::xBottom, QwtPlot::yLeft,
        // QwtPicker::PointSelection | QwtPicker::DragSelection, 
        QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn,
        canvas());
    // picker->setSelectionFlags(QwtPicker::PointSelection | QwtPicker::ClickSelection);
    picker->setStateMachine(new QwtPickerDragPointMachine());
    picker->setRubberBandPen(QColor(Qt::green));
    picker->setRubberBand(QwtPicker::CrossRubberBand);
    picker->setTrackerPen(QColor(Qt::darkBlue));
    picker->setTrackerMode(QwtPicker::ActiveOnly);
    picker->setEnabled(false);
    connect(picker, SIGNAL(selected(const QPolygon&)), this, SLOT(onPickerSelection(const QPolygon&)));
    connect(picker, SIGNAL(moved(const QPoint&)), this, SLOT(onPickerMove(const QPoint&)));

    // Avoid jumping when labels with more/less digits
    // appear/disappear when scrolling vertically

    const QFontMetrics fm(axisWidget(QwtPlot::yLeft)->font());
    QwtScaleDraw* sd = axisScaleDraw(QwtPlot::yLeft);
    sd->setMinimumExtent(fm.width("100.00"));

    const QColor c(Qt::darkBlue);
    zoomer->setRubberBandPen(c);
    zoomer->setTrackerPen(c);
}

void QSpectrogram::setInfo(const SpectrogramInfo& info) {
    QwtRasterData* data = new SpectrogramData(info);
    d_spectrogram->setData(data);
    zoomer->replaceRasterData(data);
    zoomer->setZoomBase();

    picker->setRasterData(data);
    const QwtInterval zInterval = d_spectrogram->data()->interval(Qt::ZAxis);
    setAxisScale(QwtPlot::yRight, zInterval.minValue(), zInterval.maxValue());
    enableAxis(QwtPlot::yRight);
}

void QSpectrogram::showContour(bool on) {
    d_spectrogram->setDisplayMode(QwtPlotSpectrogram::ContourMode, on);
    replot();
}
void QSpectrogram::showSpectrogram(bool on) {
    d_spectrogram->setDisplayMode(QwtPlotSpectrogram::ImageMode, on);
    d_spectrogram->setDefaultContourPen(
        on ? QPen(Qt::black, 0) : QPen(Qt::NoPen));

    replot();
}
void QSpectrogram::setColorMap(int type) {
    QwtScaleWidget* axis = axisWidget(QwtPlot::yRight);
    const QwtInterval zInterval = d_spectrogram->data()->interval(Qt::ZAxis);

    d_mapType = type;

    const QwtColorMap::Format format = QwtColorMap::RGB;

    int alpha = d_alpha;
    switch (type)     {
        case QSpectrogram::HueMap:
        {
            d_spectrogram->setColorMap(new HueColorMap(format));
            axis->setColorMap(zInterval, new HueColorMap(format));
            break;
        }
        case QSpectrogram::RGBMap:
        default:
        {
            d_spectrogram->setColorMap(new LinearColorMap(format));
            axis->setColorMap(zInterval, new LinearColorMap(format));
        }
    };
    d_spectrogram->setAlpha(alpha);

    replot();
}

void QSpectrogram::setColorTableSize(int type) {
    int numColors = 0;
    switch (type)
    {
    case 1:
        numColors = 256;
        break;
    case 2:
        numColors = 1024;
        break;
    case 3:
        numColors = 16384;
        break;
    }

    d_spectrogram->setMaxRGBTableSize(numColors);
    replot();
}
void QSpectrogram::setAlpha(int) {
}

void QSpectrogram::setAxesEqual() {
    zoomer->keepAxesEqual(true);
}

void QSpectrogram::drawItems(QPainter* painter, const QRectF& canvasRect,
    const QwtScaleMap maps[axisCnt]) const
{
    QwtPlot::drawItems(painter, canvasRect, maps);

    if (d_spectrogram)
    {
        Spectrogram* spectrogram = static_cast<Spectrogram*>(d_spectrogram);

        QString info("%1 x %2 pixels: %3 ms");
        info = info.arg(spectrogram->renderedSize().width());
        info = info.arg(spectrogram->renderedSize().height());
        info = info.arg(spectrogram->elapsed());

        QSpectrogram* sm = const_cast<QSpectrogram*>(this);
        sm->Q_EMIT rendered(info);
    }
}

void QSpectrogram::onPickerSelection(const QPolygon& pa) {

}

void QSpectrogram::onPickerMove(const QPoint& p) {

}
void QSpectrogram::enablePicker(bool e) {
    picker->setEnabled(e);
    zoomer->setEnabled(!e);
}


QSpectrogramPlot::QSpectrogramPlot(const std::string& key_, XQPlots* pf_, QWidget* parent, unsigned int flags_) : 
        JustAplot(key_, pf_, parent, jQwSpectrogram) {
    mouseMode = 0;
    flags = flags_;
    tbModeChanging = false;
    clipperHost = false;
    spectrogram = new QSpectrogram(this, flags);
    spectrogram->setObjectName(QString::fromUtf8("spectrogram"));
    spectrogram->setContentsMargins(0, 5, 0, 10);    // ????????

    setupUi();

    QIcon icon;
    icon.addPixmap(QPixmap(QString::fromUtf8(":/icons/binokl.png")), QIcon::Normal, QIcon::Off);
    icon.addPixmap(QPixmap(QString::fromUtf8(":/icons/binokl.png")), QIcon::Normal, QIcon::On);
    setWindowIcon(icon);

    connect(tbPicker, SIGNAL(toggled(bool)), this, SLOT(ontbPicker(bool)));
  //  connect(tb2, SIGNAL(toggled(bool)), this, SLOT(ontb2(bool)));
  //  connect(tb3, SIGNAL(toggled(bool)), this, SLOT(ontb3(bool)));
   // connect(tbSaveDataToTextFile, SIGNAL(clicked()), this, SLOT(onSaveData()));
    connect(tbSavePicture, SIGNAL(clicked()), this, SLOT(onSaveImage()));
    connect(tbSquareAxis, SIGNAL(toggled(bool)), this, SLOT(onTbSquareAxis(bool)));
    connect(tbClip, SIGNAL(toggled(bool)), this, SLOT(onClip(bool)));
    connect(tbResetLayout, SIGNAL(clicked()), this, SLOT(onResetLayout()));

    //panner = new QwtPlotPanner(spectrogram->canvas());
    //panner->setMouseButton(Qt::LeftButton);
    //panner->setCursor(Qt::OpenHandCursor);
    //panner->setEnabled(false);

    /*
    zoomer = new Zoomer(QwtPlot::xBottom, QwtPlot::yLeft, plot1->canvas());
    zoomer->setEnabled(true);
    zoomer->setTrackerMode(QwtPicker::ActiveOnly);
    zoomer->setTrackerPen(QColor(Qt::black));
    zoomer->zoom(0);
    */
    //tb3->setChecked(true);

    //   test: =========
   // QWidget* w1 = panner->parentWidget();
  //  QWidget* w2 = picker->parentWidget();
   // QWidget* w3 = zoomer->parentWidget();
    // zoomer->setRubberBand(QwtPicker::RectRubberBand);
    // zoomer->setRubberBandPen(QColor(Qt::green));
     //zoomer->setTrackerMode(QwtPicker::ActiveOnly);
     //zoomer->setTrackerPen(QColor(Qt::darkBlue));


 //	setCentralWidget(spectrogram);

    title(key.c_str());

    setFocusPolicy(Qt::StrongFocus);
    titleFont.setBold(false); titleFont.setPointSize(10);
    axisFont.setBold(false); axisFont.setPointSize(8);

    spectrogram->setAxisFont(0, axisFont);
    spectrogram->setAxisFont(1, axisFont);
    spectrogram->setAxisFont(2, axisFont);

    makeMarkersVisible(true);
    lastXselected = 0.0; lastYselected = 0.0;  pointWasSelected = false;
}

QSpectrogramPlot::~QSpectrogramPlot() {


}

void QSpectrogramPlot::setInfo(const SpectrogramInfo& info) {
    if (spectrogram != nullptr) {
        spectrogram->setInfo(info);
        replot();
    }
}

void QSpectrogramPlot::setupUi() {
    if (this->objectName().isEmpty())
        this->setObjectName(QString::fromUtf8("QSpectrogramPlot"));
    this->resize(640, 300);
    QFont font;
    font.setFamily(QString::fromUtf8("MS Sans Serif"));
    font.setPointSize(12);
    this->setFont(font);
    this->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
    verticalLayout = new QVBoxLayout(this);
    verticalLayout->setSpacing(2);
    verticalLayout->setMargin(2);
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));

    QToolBar* toolBar = new QToolBar(this);
    /*
    top_frame = new QFrame(this);
    top_frame->setObjectName(QString::fromUtf8("top_frame"));
    top_frame->setMinimumSize(QSize(0, 32));
    top_frame->setFrameShape(QFrame::NoFrame);
    top_frame->setFrameShadow(QFrame::Raised);
    top_frame->setLineWidth(1);
    horizontalLayout = new QHBoxLayout(top_frame);
    horizontalLayout->setSpacing(2);
    horizontalLayout->setMargin(2);
    horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
    */
    tbPicker = new QToolButton(toolBar);
    tbPicker->setObjectName(QString::fromUtf8("tbPicker"));
    ui_addTBIcon(tbPicker, ":/icons/arrow.png");
    tbPicker->setText("ARROW");
    tbPicker->setToolTip("ARROW");
    tbPicker->setCheckable(true);
    toolBar->addWidget(tbPicker);
    /*
    tb2 = new QToolButton(toolBar);
    tb2->setObjectName(QString::fromUtf8("tb2"));
    tb2->setText("PAN");
    tb2->setToolTip("PAN");

    ui_addTBIcon(tb2, ":/icons/pan.png");
    tb2->setCheckable(true);
    toolBar->addWidget(tb2);

    tb3 = new QToolButton(toolBar);
    tb3->setObjectName(QString::fromUtf8("tb3"));
    tb3->setText("ZOOM");
    tb3->setToolTip("ZOOM");

    ui_addTBIcon(tb3, ":/icons/zoom.png");
    tb3->setCheckable(true);
    toolBar->addWidget(tb3);
    */
    /*
    tbSaveDataToTextFile = new QToolButton(toolBar);
    tbSaveDataToTextFile->setToolTip("Save data to text file");
    ui_addTBIcon(tbSaveDataToTextFile, ":/icons/floppy.png");
    toolBar->addWidget(tbSaveDataToTextFile);
    */
    tbSavePicture = new QToolButton(toolBar);
    tbSavePicture->setToolTip("Save data  as image file");
    ui_addTBIcon(tbSavePicture, ":/icons/savecontent.png");
    toolBar->addWidget(tbSavePicture);

    tbSquareAxis = new QToolButton(toolBar);
    tbSquareAxis->setToolTip("make axis square");
    tbSquareAxis->setText("[]");
    tbSquareAxis->setCheckable(true);
    toolBar->addWidget(tbSquareAxis);

    tbClip = new QToolButton(toolBar);
    tbClip->setToolTip("clip");
    tbClip->setText("clip");
    ui_addTBIcon(tbClip, ":/icons/wireframe.png");
    tbClip->setCheckable(true);
    toolBar->addWidget(tbClip);

    tbResetLayout = new QToolButton(toolBar);
    tbResetLayout->setToolTip("reset zooming");
    tbResetLayout->setText("reset");
    ui_addTBIcon(tbResetLayout, ":/icons/thunderstorm-icon.png");
    //tbResetLayout->setCheckable(true);
    toolBar->addWidget(tbResetLayout);

    QComboBox* mapBox = new QComboBox(toolBar);
    mapBox->addItem("RGB");
    mapBox->addItem("Hue");
    mapBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    toolBar->addWidget(mapBox);
    connect(mapBox, SIGNAL(currentIndexChanged(int)),
        spectrogram, SLOT(setColorMap(int)));

    QCheckBox* btnSpectrogram = new QCheckBox("Spectrogram", toolBar);
    toolBar->addWidget(btnSpectrogram);
    connect(btnSpectrogram, SIGNAL(toggled(bool)),
        spectrogram, SLOT(showSpectrogram(bool)));

    QCheckBox* btnContour = new QCheckBox("Contour", toolBar);
    toolBar->addWidget(btnContour);
    connect(btnContour, SIGNAL(toggled(bool)),
        spectrogram, SLOT(showContour(bool)));

    horizontalSpacer = new QSpacerItem(244, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

   // toolBar->addItem(horizontalSpacer);
    verticalLayout->addWidget(toolBar);
    btnSpectrogram->setChecked(true);
    btnContour->setChecked(false);

    verticalLayout->addWidget(spectrogram);

    QStatusBar* statusBar = new QStatusBar(this);
    verticalLayout->addWidget(statusBar);

    connect(spectrogram, SIGNAL(rendered(const QString&)),
        statusBar, SLOT(showMessage(const QString&)),
        Qt::QueuedConnection);

    
    retranslateUi();
    QMetaObject::connectSlotsByName(this);
} // setupUi

void QSpectrogramPlot::retranslateUi() {
    //   setWindowTitle(QApplication::translate("figure2", "figure 1", 0, QApplication::UnicodeUTF8));
    //   tb1->setText(QApplication::translate("figure2", "...", 0, QApplication::UnicodeUTF8));
    //   tb2->setText(QApplication::translate("figure2", "...", 0, QApplication::UnicodeUTF8));
     //  tb3->setText(QApplication::translate("figure2", "...", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(this);
} // retranslateUi 



void QSpectrogramPlot::title(const std::string& s) {
    sTitle = s;
    QwtText title(s.c_str());
    title.setFont(titleFont);
    spectrogram->setTitle(title);
    JustAplot::title(s);
}

void QSpectrogramPlot::footer(const std::string& s) {
    QwtText foo(s.c_str());
    foo.setFont(axisFont);
    spectrogram->setFooter(foo);
}

void QSpectrogramPlot::xlabel(const std::string& s) {
    QwtText title(s.c_str());
    title.setFont(axisFont);
    spectrogram->setAxisTitle(spectrogram->xBottom, title);
}

void QSpectrogramPlot::ylabel(const std::string& s) {
    QwtText title(s.c_str());
    title.setFont(axisFont);
    spectrogram->setAxisTitle(spectrogram->yLeft, title);
}

void QSpectrogramPlot::drawMarker(double X, double Y, int type) {

}
void QSpectrogramPlot::drawMarker(double t) {

}
void QSpectrogramPlot::makeMarkersVisible(bool visible) {

    }
void QSpectrogramPlot::onClip(double t1, double t2) {

}

void QSpectrogramPlot::replot() {
    spectrogram->replot();
}

void QSpectrogramPlot::setAxesEqual() {
    
}

void QSpectrogramPlot::closeEvent(QCloseEvent* event) {

}

void QSpectrogramPlot::removeLines() {

    }
void QSpectrogramPlot::ui_addTBIcon(QToolButton* tb, const char* i) {
    QIcon icon;
    QString s = QString::fromUtf8(i);
    QPixmap pm = QPixmap(s);
    icon.addPixmap(pm, QIcon::Normal, QIcon::Off);

    //	icon.addFile(":/icons/binokl");
    tb->setIcon(icon);
}

void QSpectrogramPlot::setTBState() {

    }
void QSpectrogramPlot::onPickerSignal(int x, int y) {

    }
void QSpectrogramPlot::focusInEvent(QFocusEvent* event) {

    }
void QSpectrogramPlot::keyPressEvent(QKeyEvent* k) {

}

void QSpectrogramPlot::ontbPicker(bool checked) {
    spectrogram->enablePicker(checked);
}

void QSpectrogramPlot::onClip(bool checked) {

    }
void QSpectrogramPlot::onResetLayout() {

    }
void QSpectrogramPlot::ontb2(bool checked) {

    }
void QSpectrogramPlot::ontb3(bool checked) {

    }
void QSpectrogramPlot::onSaveData() {

    }
void QSpectrogramPlot::onSaveImage() {

    }
void QSpectrogramPlot::onTbFFT() {

    }
void QSpectrogramPlot::onTbSquareAxis(bool checked) {

    }




