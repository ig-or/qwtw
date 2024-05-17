
#include "spectrogram.h"
#include "sfigure.h"
#include <QLineEdit>
#include <qwt_color_map.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_renderer.h>
#include <qwt_picker_machine.h>
#include <qwt_interval.h>
#include <qwt_scale_engine.h>
#include <qwt_scale_map.h>
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>
#include <qpen.h>
#include <qevent.h>
#include <qtoolbar.h>
#include <qstatusbar.h>
#include <qelapsedtimer.h>
#include <qcheckbox.h>
#include "settings.h"
#include "nanoflann.hpp"
#include "qmarkers.h"

#include <limits>
#include <cmath>
#include <math.h>
#include <cfloat>

int xmprintf(int level, const char* _Format, ...);
class SpectrogramData;
class QSpectrogramPlot;


/**
Info about some 'double' parameter for one single cell.
*/
struct SpPosCoordInfo {
    double pmin, pmax;	///< max and min parameter values
    // k is the index in a sp cells 'vector'
    // y is a vertical sp cell coordinate
    // x is horizontal sp (cell) coordinate
    unsigned int xmax, xmin, ymax, ymin, kmax, kmin;
    SpPosCoordInfo();
    /// <summary>
    ///  setup everything
    /// </summary>
    void pInit();
    /**
    update info with another parameter value.
    \param p the parameter value
    \param k value for 'k min/max'
    \param k1  value for y min/max
    \param k2  value for x  min/max
    */
    void pUpdate(double p, int k, int k1, int k2);
    /**
    Check if parameter 'p' is 'inside' this cell
    \return true if inside
    */
    bool in(double p) const;
};

/**
* info about some part of the spectrogramm.
*/
struct SpCell {
    /// <summary>
    /// minimum and maximum indices of this particular spCell inside a SpectrogramInfo
    /// </summary>
    unsigned int x1, x2, y1, y2;
    /// <summary>
    ///   min amd max time for this cell
    /// </summary>
    double tMin;
    double tMax;

    unsigned int xTmax, yTmax, xTmin, yTmin, kTmin, kTmax; ///< coords corresponding to tMax and tMin
    SpPosCoordInfo px, py, pz;
};



/**
* some meta info about the spectrogramm.
*/
class SpStatInfo {
public:
    std::vector<SpCell> cells;
    /// <summary>
    /// create this meta info.
    /// </summary>
    /// <param name="si_"></param>
    /// <param name="n1"></param> approximate number of spCells to create
    SpStatInfo(const SpectrogramInfo& si, int n1 = 1000);
    //const SpCell& findT(double t);

private:
    int n = 0;

    //const SpectrogramInfo& si;
    friend void spStatInfoTest(int nx, int ny);
};

SpPosCoordInfo::SpPosCoordInfo() {
    pInit();
}
void SpPosCoordInfo::pInit() {
    pmin = DBL_MAX;
    pmax = -DBL_MAX;
    xmax = 0;
    xmin = 0;
    ymax = 0;
    ymin = 0;
    kmax = 0;
    kmin = 0;
}
void SpPosCoordInfo::pUpdate(double p, int k, int k1, int k2) {
    if (pmax < p) {
        pmax = p;
        xmax = k2;
        ymax = k1;
        kmax = k;
    }
    if (pmin > p) {
        pmin = p;
        xmin = k2;
        ymin = k1;
        kmin = k;
    }
}
bool SpPosCoordInfo::in(double p) const {
    if (pmax >= p && pmin <= p) {
        return true;
    } else {
        return false;
    }
}

SpStatInfo::SpStatInfo(const SpectrogramInfo& si, int n1) {
    double a1 = sqrt((si.nx * si.ny) / ((double)(n1))); //  how many 'ponts' in one spCell ??    sqrt
    int a = std::roundl(a1);
    if (a < 1) {
        a = 1;
    }
    int nx = floor(si.nx / a) + 1;
    int ny = floor(si.ny / a) + 1;
    n = nx * ny;             // real number of spCell
    int i, j, k1, k2, k, ky, i1;
    unsigned int x1 = 0, x2 = 0, y1 = 0, y2 = 0;
    double tMin = 0.0;
    double tMax = 0.0;
    double t;
    unsigned int xTmax, yTmax, xTmin, yTmin, kTmin, kTmax;
    SpPosCoordInfo px, py, pz;
    double* pp = 0;
    cells.resize(n);
    y1 = 0;
    for (i = 0; i < ny; i++) {
        y2 = y1 + a - 1;
        if (y2 >= si.ny) {
            assert(i == ny - 1);
            y2 = si.ny - 1;
            assert(y2 > y1);
        }
        x1 = 0;
        i1 = i * nx;
        for (j = 0; j < nx; j++) {
            x2 = x1 + a - 1;
            if (x2 >= si.nx) {
                assert(j == nx - 1);
                x2 = si.nx - 1;
                assert(x2 > x1);
            }
            tMax = -DBL_MAX;
            tMin = DBL_MAX;
            xTmax = 0; yTmax = 0; xTmin = 0; yTmin = 0; kTmin = 0;  kTmax = 0;
            px.pInit(); py.pInit(); pz.pInit();
            for (k1 = y1; k1 <= y2; k1++) {
                ky = k1 * si.nx;
                for (k2 = x1; k2 <= x2; k2++) {
                    k = ky + k2;
                    if (si.t != nullptr) {
                        t = si.t[k];
                        if (tMax < t) {
                            tMax = t;
                            xTmax = k2;
                            yTmax = k1;
                            kTmax = k;
                        }
                        if (tMin > t) {
                            tMin = t;
                            xTmin = k2;
                            yTmin = k1;
                            kTmin = k;
                        }
                    }
                    if (si.p != nullptr) {
                        pp = si.p + k * 3;    //  this particular point
                        px.pUpdate(pp[0], k, k1, k2);
                        py.pUpdate(pp[1], k, k1, k2);
                        pz.pUpdate(pp[2], k, k1, k2);
                    }
                }
            }
            cells[i1 + j] = SpCell{ x1, x2, y1, y2, tMin, tMax, xTmax, yTmax, xTmin, yTmin, kTmin, kTmax, px, py, pz};
            x1 = x2 + 1;
        }
        y1 = y2 + 1;
    }
    assert(x2 == si.nx - 1);
    assert(y2 == si.ny - 1);
    assert(x2 > x1);
    assert(y2 > y1);
}
/*
const SpCell& SpStatInfo::findT(double t) {
    for (const auto& a : cells) {

    }
    return cells[0];
}
*/

void spStatInfoTest(int nx, int ny) {
    SpectrogramInfo si;
    si.nx = nx;
    si.ny = ny;
    si.xmin = 0.0;
    si.xmax = 10.0;
    si.ymin = -0.1;
    si.ymax = 15.0;

    si.p = 0;
    si.t = new double[si.nx * si.ny];
    int i, j, k;
    for (i = 0; i < nx; i++) {
        for (j = 0; j < ny; j++) {
            k = j * nx + i;
            si.t[k] = i + j;
        }
    }
    SpStatInfo test(si, 1000);
    xmprintf(1, "test.n = %d \n", test.n);
}

class FSPicker2 : public QwtPlotPicker {
public:
    FSPicker2(int xAxis, int yAxis, RubberBand rubberBand,
        DisplayMode trackerMode, QWidget*);
    QPointF transform1(const QPoint& pos) 	 const;
    void setRasterData(SpectrogramData* rd_) {
        rd = rd_;
    }

protected:
    //QwtRasterData* rd = nullptr;
    SpectrogramData* rd = nullptr;

    virtual QwtText trackerTextF(const QPointF& pos) const;

};


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

class SpectrogramData;

//  this is for nanoflann
using my_kd_tree_t = nanoflann::KDTreeSingleIndexAdaptor<
    nanoflann::L2_Simple_Adaptor<double, SpectrogramData>, SpectrogramData, 3 /* dim */
>;

class SpectrogramData : public QwtRasterData
{
public:
    double dx, dy;                  //      size of one pixel
    double zMin, zMax, xMin, xMax, yMin, yMax, wx, wy;      // 
    double* p = nullptr;   ///< a 'point info' for every cell
    double* tt = nullptr;   ///< a 'time info' for every  cell
    my_kd_tree_t* pointsTree = nullptr;


    SpectrogramData(const SpectrogramInfo& info) : statInfo(info) {
        // some minor performance improvements when the spectrogram item
        // does not need to check for NaN values
        setAttribute(QwtRasterData::WithoutGaps, true);
        
        if ((info.nx < 2) || (info.ny < 2) || (info.xmin > info.xmax) || (info.ymin > info.ymax) || (info.z == 0)) {
            xmprintf(1, "SpectrogramData(const SpectrogramInfo& info): ERROR in parameters \n");
            return;
        }

        //  copy the data:
        xSize = info.nx;
        ySize = info.ny;
        nSize = xSize * ySize;
        int nz = nSize;
        try {
            //yData = new double[info.ny];
            zData = new double[nz];
            //xData = new double[info.nx];
        } catch (std::exception& ex) {
            xmprintf(1, "SpectrogramData(const SpectrogramInfo& info): ERROR in memory allocation (1) (%s) \n", ex.what());
            return;
        }
        if (zData == nullptr) {
            xmprintf(1, "SpectrogramData(const SpectrogramInfo& info): ERROR in memory allocation (4)  \n");
            return;
        }
        //memcpy(xData, info.x, xSize * sizeof(double));
        //memcpy(yData, info.y, ySize * sizeof(double));
        memcpy(zData, info.z, nz * sizeof(double));             //  hope that info.z haz correct size

        //  save intervals
        xMin = info.xmin; xMax = info.xmax;
        yMin = info.ymin; yMax = info.ymax;
        d_intervals[Qt::XAxis] = QwtInterval(xMin, xMax);
        d_intervals[Qt::YAxis] = QwtInterval(yMin, yMax);

        // calculate z min and z max..   this might take some time
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

        try {
            if ((info.p != 0)) {
                p = new double[nz * 3];
                if ((p != nullptr)) {
                    memcpy(p, info.p, nz * sizeof(double) * 3);
                } else {
                    xmprintf(1, "SpectrogramData(const SpectrogramInfo& info): ERROR in memory allocation (3) (out of memory?)\n");
                }
                
                //  this is for nanoflann
                pointsTree = new my_kd_tree_t(3 /*dim*/, *this, { 50 /* max leaf */ });
            }
            if ((info.t != 0)) {
                tt = new double[nz];
                if ((tt != nullptr)) {
                    memcpy(tt, info.t, nz * sizeof(double));
                } else {
                    xmprintf(1, "SpectrogramData(const SpectrogramInfo& info): ERROR in memory allocation (4) (out of memory?)\n");
                }
            }
        } catch (std::exception& ex) {
            xmprintf(1, "SpectrogramData(const SpectrogramInfo& info): ERROR in memory allocation (2) (%s) \n", ex.what());
            return;
        }
    }

    //  this is for nanoflann
    int do_knn_search(double* x) const {
        if (p == nullptr || x == nullptr) {
            return -1;
        }
        // do a knn search
        const size_t                   num_results = 1;
        size_t                         ret_index;
        double                          out_dist_sqr;
        nanoflann::KNNResultSet<double> resultSet(num_results);
        double* query_pt = x;
        resultSet.init(&ret_index, &out_dist_sqr);
        pointsTree->findNeighbors(resultSet, &query_pt[0]);
        if (num_results > 0) {
            return ret_index;
        }
        return -1;
    }

    virtual QwtInterval interval(Qt::Axis axis) const QWT_OVERRIDE
    {
        if (axis >= 0 && axis <= 2)
            return d_intervals[axis];

        return QwtInterval();
    }

    // Must return the number of data points
   //  this is for nanoflann
    inline size_t kdtree_get_point_count() const  {
        if (p == nullptr) {
            return 0;
        } else {
            return nSize;
        }
    }

    // Returns the dim'th component of the idx'th point in the class
    //  this is for nanoflann
    inline double kdtree_get_pt(const size_t idx, const size_t dim) const
    {
        if (p == nullptr) {
            return 0.0;
        } else {
            return p[idx * 3 + dim];
        }
    }

    // Optional bounding-box computation: return false to default to a standard
    // bbox computation loop.
    //   Return true if the BBOX was already computed by the class and returned
    //   in "bb" so it can be avoided to redo it again. Look at bb.size() to
    //   find out the expected dimensionality (e.g. 2 or 3 for point clouds)
    //  this is for nanoflann
    template <class BBOX> bool kdtree_get_bbox(BBOX& /*bb*/) const
    {
        return false;
    }


    /**
    * \param[out] iy    vertical index 
    * \param[out] ix    horizontal index 
    * \param[out] z the spectrogramm value
    * \return true if OK
    */
    bool d2i(double x, double y, int& ix, int& iy, double& z) const {
        ix = 0;
        iy = 0;
        z = 0.0;
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
        z = zData[iy * xSize + ix];
        return true;
    }

    virtual double value(double x, double y) const QWT_OVERRIDE
    {
        if ((zData == 0) || (xSize == 0)) {
            return 0;
        }
        int ix = 0;
        int iy = 0;
        double z;
        bool test = d2i(x, y, ix, iy, z);
        return z;
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
        if (p != nullptr) {
            delete[] p; p = nullptr;
        }
        if (tt != nullptr) {
            delete[] tt; tt = nullptr;
        }
        if (pointsTree != nullptr) {
            delete pointsTree;
            pointsTree = nullptr;
        }
    }

    /** 
    * \param[in] i 'vertical' index (y)
    * \param[in] j (x)
    * \return the 't' from the indices of the spectrogramm.
    */
    double getT(int i, int j) {
        if (tt == 0) return 0.0;
        unsigned int k = i * xSize + j;
        if (k >= xSize * ySize) {
            return 0.0;
        }
        return tt[k];
    }

    /**
    * \param[in] i row index  
    * \param[in] j column index
    * \return a 'point info' from spectrogramm coords.
    
    */
    bool getP(int i, int j, double* dst) {
        if (p == 0) return false;
        unsigned int k = i * xSize + j;
        if (k >= xSize * ySize) {
            return false;
        }
        memcpy(dst, p + k * 3, sizeof(double) * 3);
        return true;
    }

    /** find coords and indices from time info.
    * 
    * \param[out] xx horizontal coord
    * \param[out] yy vertical coord
    * \param[out] jj horizontal index     (col #)
    * \param[out] ii vertical index      (row #)
    \return true if all is OK
    */
    bool findT(double t, double& xx, double& yy, int& jj, int& ii) {
        if (tt == 0) {
            return false;
        }
        xx = 0.0;
        yy = 0.0;
        ii = 0;
        jj = 0;
        double mdt = DBL_MAX;
        double dt;
        int i, j, k, raw;
        bool u = false;
        for (const auto& a : statInfo.cells) {              //  for every 'cells'
            if (a.tMax >= t && a.tMin <= t) {               //   if 't' might be inside
                for (i = a.y1; i <= a.y2; i++) {            //  check all the rows of this cell
                    raw = i * xSize;
                    for (j = a.x2; j <= a.x2; j++) {        //  check all the columns of this cell
                        k = raw + j;
                        dt = fabs(tt[k] - t);
                        if (mdt > dt) {                     //  remember the minimum 'time distance'
                            mdt = dt;
                            ii = i;
                            jj = j;
                            u = true;
                        }
                    }
                }
            } else { // out of bounds for this cell, but anyway:
                dt = fabs(tt[a.kTmin] - t);
                if (mdt > dt) {
                    mdt = dt;
                    ii = a.yTmin;
                    jj = a.xTmin;
                    u = true;
                }
                dt = fabs(tt[a.kTmax] - t);
                if (mdt > dt) {
                    mdt = dt;
                    ii = a.yTmax;
                    jj = a.xTmax;
                    u = true;
                }
            }
        }
        if (!u) {
            return false;
        }
        xx = dx * jj + xMin;
        yy = dy * ii + yMin;

        return true;
    }


    bool findP(double* pp, double& xx, double& yy, int& jj, int& ii) {
        if (p == nullptr) {
            return false;
        }
        int k = do_knn_search(pp);
        if (k < 0 || k >= nSize) {
            return false;
        }
        jj = k % xSize;
        ii = std::lround((k - jj) / xSize);

        xx = dx * jj + xMin;
        yy = dy * ii + yMin;
        return true;
    }


    ~SpectrogramData() {
        clearData();
    }
private:
    QwtInterval d_intervals[3];
    int xSize = 0;          /// number of horizontal points
    int ySize = 0;          /// number of vertical points
    int nSize = 0;          /// xSize * ySize
    //double* xData = 0;      // hor points 
    //double* yData = 0;      // ver points
    double* zData = nullptr;        // the[ xSize x ySize] matrix of all the points


    SpStatInfo statInfo;
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
    int ix = 0;
    int iy = 0;
    double t = 0.0;
    if (rd != nullptr) {
        rd->d2i(x, y, ix, iy, z);
        t = rd->getT(iy, ix);
    }
    char stmp[256];
    snprintf(stmp, 256, "%.3f (%.3f, %.3f)[%d %d] %.3f", z, x, y, ix, iy, t);
    stmp[255] = 0;
    QwtText text = QString::fromUtf8(stmp);
    text.setBackgroundBrush(QBrush(bg));
    QFont font("Helvetica [Cronyx]", 12);
    text.setFont(font);
    return text;
}


class MyZoomer : public QwtPlotZoomer {
public:
    MyZoomer(QWidget* canvas, SpectrogramData* rd_) :
        QwtPlotZoomer(canvas), rd(rd_)
    {
        setTrackerMode(AlwaysOn);
    }

    virtual QwtText trackerTextF(const QPointF& pos) const QWT_OVERRIDE {
        QColor bg(Qt::white);
        bg.setAlpha(185);
        double x = pos.x();
        double y = pos.y();
        double z = 0.0;
        int ix = 0, iy = 0;
        double t = 0.0;
        if (rd != 0) {
            rd->d2i(x, y, ix, iy, z);
            t = rd->getT(iy, ix);
        }
        char stmp[256];
        snprintf(stmp, 256, "%.3f (%.3f, %.3f)[%d %d] %.3f", z, x, y, ix, iy, t);
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
    void replaceRasterData(SpectrogramData* rd_) {
        rd = rd_;
    }
    virtual QRect trackerRect(const QFont& f) const QWT_OVERRIDE {
        QRect rect = QwtPicker::trackerRect(f);

        return rect;
    }
protected:
    void rescale() {
        //QwtPlotZoomer::rescale();
        //return;

        QwtPlot* plt = plot();
        if (!plt)
            return;

        const QRectF& rect = zoomRect();
        if (rect != scaleRect())
        {
            const bool doReplot = plt->autoReplot();
            plt->setAutoReplot(false);

            double x1 = rect.left();
            double x2 = rect.right();
            double y1 = rect.top();
            double y2 = rect.bottom();

            if (shouldKeepAxesEqual == false) {
                if (!plt->axisScaleDiv(xAxis()).isIncreasing())
                    qSwap(x1, x2);
                plt->setAxisScale(xAxis(), x1, x2);

                if (!plt->axisScaleDiv(yAxis()).isIncreasing())
                    qSwap(y1, y2);
                plt->setAxisScale(yAxis(), y1, y2);
            } else {
                QwtScaleMap smY = plt->canvasMap(QwtPlot::yLeft);
                QwtScaleMap smX = plt->canvasMap(QwtPlot::xBottom);

                double xScale = smX.pDist() / fabs(x2 - x1);
                double yScale = smY.pDist() / fabs(y2 - y1);

                if (xScale < yScale) { //  change Y scale
                    double ysMiddle = (y1 + y2) * 0.5;
                    double dy = (smY.pDist() / xScale) * 0.5;
                    plt->setAxisScale(QwtPlot::yLeft, ysMiddle - dy, ysMiddle + dy);
                    plt->setAxisScale(QwtPlot::xBottom, x1, x2);
                } else { //  change X scale
                    double xsMiddle = (x1 + x2) * 0.5;
                    double dx = (smX.pDist() / yScale) * 0.5;
                    plt->setAxisScale(QwtPlot::xBottom, xsMiddle - dx, xsMiddle + dx);
                    plt->setAxisScale(QwtPlot::yLeft, y1, y2);
                }
            }

            plt->setAutoReplot(doReplot);

            plt->replot();
        }
    }

private:
    bool shouldKeepAxesEqual = false;
    SpectrogramData* rd = nullptr;
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
    spp = (QSpectrogramPlot*)parent;
    d_spectrogram = new Spectrogram();
    d_spectrogram->setRenderThreadCount(0); // use system specific thread count
    d_spectrogram->setCachePolicy(QwtPlotRasterItem::PaintCache);
    legend = "spectrogramm";
    QList<double> contourLevels;
    for (double level = 0.5; level < 10.0; level += 1.0)
        contourLevels += level;
    d_spectrogram->setContourLevels(contourLevels);
    QwtRasterData* data = new TestSpectrogramData();
    d_spectrogram->setData(data);
    d_spectrogram->attach(this);
    lastXselected = 0.0; lastYselected = 0.0;

    const QwtInterval zInterval = d_spectrogram->data()->interval(Qt::ZAxis);

    // A color bar on the right axis
    QwtScaleWidget* rightAxis = axisWidget(QwtPlot::yRight);
    rightAxis->setTitle("Intensity");
    rightAxis->setColorBarEnabled(true);

    setAxisScale(QwtPlot::yRight, zInterval.minValue(), zInterval.maxValue());
    enableAxis(QwtPlot::yRight);

    plotLayout()->setAlignCanvasToScales(true);

    setColorMap(QSpectrogram::RGBMap);

    // zoom in/out with the wheel
    (void) new QwtPlotMagnifier(canvas());

    // LeftButton for the zooming
    // MidButton for the panning
    // RightButton: zoom out by 1
    // Ctrl+RighButton: zoom out to full size

    zoomer = new MyZoomer(canvas(), nullptr);
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

    ma = new QwtPlotMarker();
    ma->setVisible(false);
    maIsVisible = false;
    QPen pen;
    QColor color = Qt::red;
    pen.setColor(QColor(Qt::darkBlue));
    pen.setWidth(3);
    QwtSymbol* pms = new QwtSymbol();
    pms->setPen(pen);
    pms->setStyle(QwtSymbol::Diamond);
    pen.setColor(Qt::darkBlue);
    pms->setSize(10, 10);
    pms->setBrush(color);
    ma->setSymbol(pms);
    ma->setXValue(100.0);
    ma->setYValue(50.0);
    ma->attach(this);

    replot();
}

QSpectrogram::~QSpectrogram() {
    //delete d_spectrogram;   this supposed to be deleted automatically, since it was 'attached' 
    
//    if (ma != 0) {
//        delete ma;
//    }

}

void QSpectrogram::setInfo(const SpectrogramInfo& info) {
    if (d_spectrogram == nullptr) {
        return;
    }
    sData = new SpectrogramData(info);  //  this would COPY (memcpy) all the info from 'info' into the 'data'
    QwtRasterData* data = sData;
    d_spectrogram->setData(data);           //  data hopefully would be deleted in d_spectrogram  destructor
    zoomer->replaceRasterData(sData);
    zoomer->setZoomBase();

    picker->setRasterData(sData);
    const QwtInterval zInterval = d_spectrogram->data()->interval(Qt::ZAxis);
    setAxisScale(QwtPlot::yRight, zInterval.minValue(), zInterval.maxValue());
    enableAxis(QwtPlot::yRight);
    setColorMap(QSpectrogram::RGBMap);
}

bool QSpectrogram::haveTimeInfo() const {
    return sData->tt != nullptr;
}
bool QSpectrogram::havePointsInfo() const {
    return sData->p != nullptr;
}

void QSpectrogram::drawMarker3D(double* x) {
    double xx, yy;
    int ii, jj;
    if (sData && sData->findP(x, xx, yy, ii, jj)) {
        ma->setValue(xx, yy);
        if (!maIsVisible) {
            maIsVisible = true;
            ma->setVisible(true);
        }
    }
}

void QSpectrogram::drawMarker(double t) {
    //ma->setValue(0.0, 0.0);
    double xx, yy;
    int ii, jj;
    if (sData && sData->findT(t, xx, yy, ii, jj)) {
        ma->setValue(xx, yy);
        if (!maIsVisible) {
            maIsVisible = true;
            ma->setVisible(true);
        }
    }
    //replot();
}

void QSpectrogram::onPickerSignal(int x, int y) {
    if (sData == nullptr) {
        return;
    }
    
    QPointF p = picker->transform1(QPoint(x, y));
    double xx = p.x();
    double yy = p.y();

    int ix, iy;
    double z;
    
    sData->d2i(xx, yy, ix, iy, z);
    lastXselected = xx;
    lastYselected = yy;

    if (sData->tt != nullptr) {
        double t = sData->getT(iy, ix);
        spp->picker_t(t);
    }
    if (sData->p != nullptr) {
        double point[3];
        if (sData->getP(iy, ix, point)) {
            spp->picker_p(point);
        }
    }
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
void QSpectrogram::resizeEvent(QResizeEvent* e) {
    QwtPlot::resizeEvent(e);
    if (squareAxis) {
        doSquareAxis();
    }
}
void QSpectrogram::setAxisSquare(bool square) {
    squareAxis = square;
    if (squareAxis) {
        doSquareAxis();
    }
    zoomer->keepAxesEqual(square);
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
    int x = pa.at(0).x();
    int y = pa.at(0).y();

    onPickerSignal(x, y);
}

void QSpectrogram::onPickerMove(const QPoint& p) {
    onPickerSignal(p.x(), p.y());
}

void QSpectrogram::enablePicker(bool e) {
    picker->setEnabled(e);
    zoomer->setEnabled(!e);
}

void QSpectrogram::doSquareAxis() {
    const bool doReplot = autoReplot();
    setAutoReplot(false);

    QwtScaleMap smY = canvasMap(QwtPlot::yLeft);
    QwtScaleMap smX = canvasMap(QwtPlot::xBottom);

    double qsx1 = sData->xMin;
    double qsx2 = sData->xMax;
    double qsy1 = sData->yMin;
    double qsy2 = sData->yMax;


    double dxS = smX.sDist();
    double dyS = smY.sDist();

    dxS = qsx2 - qsx1;
    dyS = qsy2 - qsy1;

    double pxS = smX.pDist();
    double pyS = smY.pDist();

    double xScale = pxS / dxS;
    double yScale = pyS / dyS;


    if (xScale < yScale) { //  change Y scale
        double s1 = smY.s1();
        double s2 = smY.s2();

        s1 = qsy1;
        s2 = qsy2;

        double ysMiddle = (s1+s2) * 0.5;
        double dy = (pyS / xScale) * 0.5;
        setAxisScale(QwtPlot::yLeft, ysMiddle - dy, ysMiddle + dy);
    } else { //  change X scale
        double s1 = smX.s1();
        double s2 = smX.s2();

        s1 = qsx1;
        s2 = qsx2;

        double xsMiddle = (s1 + s2) * 0.5;
        double dx = (pxS / yScale) * 0.5;
        setAxisScale(QwtPlot::xBottom, xsMiddle - dx, xsMiddle + dx);
    }

    setAutoReplot(doReplot);
    replot();
}





QSpectrogramPlot::QSpectrogramPlot(const std::string& key_, XQPlots* pf_, QWidget* parent, unsigned int flags_) : 
        JustAplot(key_, pf_, parent, jQwSpectrogram) {
    mouseMode = 2;
    flags = flags_;
    tbModeChanging = false;
    clipperHost = false;
    legend = "QSpectrogramPlot";
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
   // lastXselected = 0.0; lastYselected = 0.0;  
    pointWasSelected = false;
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

int QSpectrogramPlot::savePng(const std::string& filename) {
    QPixmap qPix = spectrogram->grab();
    if (qPix.isNull()) {
        return 6;
    }
    qPix.save(filename.c_str(), "png");

    return 0;
}

bool QSpectrogramPlot::haveTimeInfo() const {
    return spectrogram->haveTimeInfo();
}
bool QSpectrogramPlot::havePointsInfo() const {
    return spectrogram->havePointsInfo();
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

void QSpectrogramPlot::draw3DMarker(const CBPickerInfo& cpi) {
  //  if (cpi.plotID == iKey) { // should I react on my own picker ????  YES!
  //      return;
  //  }
    double x[3];
    x[0] = cpi.x;
    x[1] = cpi.y;
    x[2] = cpi.z;
    spectrogram->drawMarker3D(x);
}

void QSpectrogramPlot::drawMarker(double X, double Y, int type) {

}
void QSpectrogramPlot::drawMarker(double t) {
    spectrogram->drawMarker(t);
}
void QSpectrogramPlot::makeMarkersVisible(bool visible) {

    }
void QSpectrogramPlot::onClip(double t1, double t2) {

}

void QSpectrogramPlot::replot() {
    spectrogram->replot();
}
/*
void QSpectrogramPlot::setAxesEqual() {
    squareAxis = square;
    if (squareAxis) {
        doSquareAxis();
    }
}
*/

void QSpectrogramPlot::closeEvent(QCloseEvent* event) {

}
void QSpectrogramPlot::picker_t(double t) {
    CBPickerInfo cbi1;
    cbi1.index = 0;
    cbi1.lineID = 0;
    cbi1.plotID = iKey;
    cbi1.time = t;
    cbi1.type = 1;
    cbi1.x = 0;
    cbi1.y = 0;
    cbi1.xx = 0;
    cbi1.yy = 0;
    cbi1.z = 0.0;
    cbi1.flag = 1;   // only time info
    pointWasSelected = true;

    pf->drawAllMarkers2(cbi1);
}

void QSpectrogramPlot::picker_p(double* p) {
    //pf->draw3DpointMArker(iKey, p);
    pointWasSelected = true;
    pf->on3DMarker(p);
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


int QSpectrogramPlot::markerTest(int& mid, std::string& label, QColor& color) {
    int ret = 0;
    mid = 0;
    if (mouseMode != 1) {
        xmprintf(3, "QSpectrogramPlot::markerTest(): mouseMode = %d \n", mouseMode);
        return 0;
    }
    if (!pointWasSelected) {
        xmprintf(3, "QSpectrogramPlot::markerTest(): point was not selected \n");
        return 0;
    }

    xmprintf(3, "QSpectrogramPlot::markerTest(): ..... \n");
    bool haveItAlready = false;
    QwtScaleMap smY = spectrogram->canvasMap(QwtPlot::yLeft);
    QwtScaleMap smX = spectrogram->canvasMap(QwtPlot::xBottom);
    double dxS = smX.sDist();
    double dyS = smY.sDist();

    double dx = dxS / 256.0;
    double dy = dyS / 100.0;


    for (AMarker* a : amList) {
        if ((fabs(spectrogram->lastXselected - a->x) < dx) && (fabs(spectrogram->lastYselected - a->y) < dy)) {
            haveItAlready = true;
            mid = a->id;
            break;
        }
    }

    if (haveItAlready) {
        return 2;
    }
    char tmp[64];

    snprintf(tmp, 64, "[%.2f, %.2f]", spectrogram->lastXselected, spectrogram->lastYselected);
    {
        SelectMarkerParamsDlg dlg(this, tmp);
        dlg.exec();
        if (!dlg.ret) {
            xmprintf(3, "Figure2::markerTest(): rejected \n");
            return 0;
        }
        label = dlg.text->text().toStdString();
        color = dlg.selectedColor;
    }
    return 1;
}

void QSpectrogramPlot::addAMarker() {
    int mid = 0;
    std::string label;
    QColor color;
    AMarker* am;
    int  test = markerTest(mid, label, color);
    switch (test) {
    case 1:
        markerID++;
        am = new AMarker(label.c_str(), spectrogram->lastXselected, spectrogram->lastYselected, color,
            static_cast<AMarker::AMPos>(qwSettings.direction), markerID);
        amList.push_back(am);
        am->attach(spectrogram);
        break;
    case 2:
    {
        std::list<AMarker*>::iterator it = amList.begin();
        while (it != amList.end()) {
            AMarker* a = *it;
            if (a->id == mid) {
                a->detach();
                delete a;
                it = amList.erase(it);
            } else {
                ++it;
            }
        }
    }
    break;
    };
    replot();
    xmprintf(3, "Figure2::addAMarker(): OK \n");
}


void QSpectrogramPlot::keyPressEvent(QKeyEvent* k) {
    switch (k->key()) {
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_Left:
    case Qt::Key_Right:
        QWidget::keyPressEvent(k);
        break;
    case Qt::Key_A:  //  add arrow marker
        xmprintf(9, "QSpectrogramPlot:: A was pressed!\n");
        addAMarker();
        break;
    case Qt::Key_M: //  switch to marker mode
        mouseMode = 1;
        //setTBState();
        break;
    case Qt::Key_P: //  switch to pan mode
        mouseMode = 2;
        //setTBState();
        break;
    case Qt::Key_Z: //  switch to zoom mode
        mouseMode = 3;
        //setTBState();
        break;
    default:
        QWidget::keyPressEvent(k);
        break;
    };
    return;
}

void QSpectrogramPlot::ontbPicker(bool checked) {
    spectrogram->enablePicker(checked);
    mouseMode = checked ? 1 : 2;
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
    spectrogram->setAxisSquare(checked);
}




