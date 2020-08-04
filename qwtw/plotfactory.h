
/**

	Main library interface file.
	Wrapper for all the functions.
	

	\file plotfactory.h
	\author   Igor Sandler
	\date    Jul 2009
	\version 1.0
	
*/

 

#ifndef PLOTFACTORY_H_FILE
#define PLOTFACTORY_H_FILE

class QWidget;
class Gra1;

#include <QObject>
//#include <qwt_polygon.h>

#ifdef qwtflib_EXPORTS	
#define qwtflib_API Q_DECL_EXPORT //__declspec(dllexport)
#else
#define qwtflib_API Q_DECL_IMPORT // __declspec(dllimport)
#endif


/** \class PlotFactory
	Plot factory can draw 2D  and sometimes 3D plots.

	plot usage example:
	\code
		PlotFactory pf;
		pf.figure(35);
		pf.title("yaw estimation");
		pf.xlabel("time [s]");
		pf.ylabel("[rad] [m/s]");

		int hmGps = 100;
		int W = 4, i;
		double* yaw_stat = new double[hmGps*W];
		for (i = 0; i < hmGps; i++) {
			yaw_stat[i] = i / 10.; 
			yaw_stat[i + 1*hmGps] = sin(yaw_stat[i]);
			yaw_stat[i + 2*hmGps] = sin(yaw_stat[i]) + cos(yaw_stat[i]);
			yaw_stat[i + 3*hmGps] = sin(yaw_stat[i]) / 1.84 + 0.8;
		}
		
		pf.plot(yaw_stat, yaw_stat + 1*hmGps, hmGps, "gps azimut", "-eb");
		pf.plot(yaw_stat, yaw_stat + 2*hmGps, hmGps, "plane velocity", "-dm");
		pf.plot(yaw_stat, yaw_stat + 3*hmGps, hmGps, "smoothed azimut", "-sr");
	\endcode

*/
class   PlotFactory: public QObject  {
	Q_OBJECT
	
public:
	/** A constructor. 
	@param[in] parent1    put your container class pointer here
	*/
	qwtflib_API PlotFactory(QWidget * parent1 = 0);
	/**  destructor */
	qwtflib_API ~PlotFactory();

	/**    create a 2D plot with a number, or raise up  already created plot  
		@param[in] n a plot number. There can be only one plot with this number in PlotFactory.
	*/
	qwtflib_API void figure(int n);
	/**    create a 3D plot with a number, or raise up  already created plot  
		@param[in] n a plot number. There can be only one plot with this number in PlotFactory.
	*/

	//qwtflib_API void figure3(int n);
	/**   set current plot title  
	@param[in] s plot title
	*/
	qwtflib_API void title(char* s);
	/**  set current plot X axes label  
		@param[in] s axes label.
	*/
	qwtflib_API void xlabel(char* s);
	/**  set current plot Y axes label  
		@param[in] s axes label.
	*/
	qwtflib_API void ylabel(char* s);
	/**  set current plot Z axes label  
	This is working for 3D plots only.
		@param[in] s axes label.
	*/
	qwtflib_API void zlabel(char* s);
	/**  plot 2D line  on the current plot.


		@param[in]  x   x-data
		@param[in]  y	y-data
		@param[in]  size size of x and y
		@param[in]  name line name (used for a legend )
		@param[in]  style line style.
		This is a string with 1, 2 or 3 letters.
		 - Last symbol is always line color.
		 \code
		'r' : Qt::red 
		'd' dark red
		'k' : Qt::black
		'w' : Qt::white
		'g' : Qt::green
		'G' : dark green
		'm' : Qt::magenta
		'M' : dark magneta
		'y' : Qt::yellow
		'Y'  : darkYellow
		'b' : Qt::blue
		'c'  cyan
		'C' darkCyan
		\endcode

		- first is always a line style:
		 \code
		' ': QwtPlotCurve::NoCurve
		'-': QwtPlotCurve::Lines
		'%': QwtPlotCurve::Sticks
		'#': QwtPlotCurve::Steps
		'.': QwtPlotCurve::Dots 
		\endcode

		-    middle is symbol type
		 \code
		'e':QwtSymbol::Ellipse
		'r':QwtSymbol::Rect  
		'd':QwtSymbol::Diamond
		't':QwtSymbol::Triangle
		'x':QwtSymbol::Cross 
		's':QwtSymbol::Star1 
		'q':QwtSymbol::Star2 
		'w':QwtSymbol::XCross
		'u':QwtSymbol::UTriangle
		\endcode

		@param[in]  lineWidth  line width
		@param[in]  symSize symbol size
	*/
	qwtflib_API void plot(double* x, double* y, int size,  char* name = 0, const char* style = 0, int lineWidth = 1, int symSize = 8);
	/**  Draw 3D line plot.
		@param p	all the points 
		p["point number" + "coord index, 0..2" * N]
		@param N	number of points
		@param[in] res resolution
		@param r	radius for every point
		@param c	color for every point
	*/

	qwtflib_API void plot3(double* p, int N, double res, double* r = 0, double* c = 0);
	/**  set a legend for 3D plot  
	
	*/
	qwtflib_API void legend(char* text, double min, double max, int orientation = 1);
	/** Call this function before program exit.

	*/
	qwtflib_API void onExit();

signals:
	qwtflib_API void select(int h, int index);

private:
	Gra1* gra;
	int lastFigureType;

private slots:
#ifdef qwtflib_EXPORTS	
	void selection(int ha, int index);
#else
#ifndef Q_MOC_RUN
	void selection(int ha, int index);
#endif
#endif
};


#endif



