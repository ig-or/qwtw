#pragma once

//#include <mgl2/qmathgl.h>
#include <mgl2/wnd.h>
#include <mgl2/canvas.h>
#include <mgl2/qmathgl.h>


//
//#include <QFrame>
#include <QWidget>
#include <QGLWidget>
#include <memory>

class QMathGL;
class mglGraph;
class QSpinBox;
class QFrame;
class QMenu;
class QTimer;
class QMenuBar;
class QBoxLayout;
class QResizeEvent;
class QScrollArea;


struct ARange {
	double xMin, xMax, yMin, yMax, zMin, zMax;
	ARange();
	ARange(int size, double* x, double* y, double* z);
	bool update(const ARange& r);
};

struct ThreeDline {
	ARange range;
   	mglData mx;
	mglData my;
	mglData mz;
	std::string style;
	ThreeDline(int size, double* x, double* y, double* z, const std::string& style_);
};

struct SurfData {
	mglData f;
	ARange range;
	std::string style;

	/** 
	 * \param data double array [xSize x ySize]; point M(x[i], y[j]) have data[i + xSize*j]
	 * */
	SurfData(int xSize, int ySize, double xMin, double xMax, double yMin, double yMax, double* data, const std::string& style_);
};

class AnotherDraw : public mglDraw {
	ARange range;
	//mglGraph *gr;
	std::list<ThreeDline> lines;
	std::list<std::shared_ptr<SurfData>> surfs;

	int drawCounter;
	int endOfResizeFlag;
	int plotsCount;

public:
	std::string xLabel, yLabel, zLabel;
	//std::string title;
	bool useBox, useGrid;

	void onResize();
	AnotherDraw();
	~AnotherDraw();
	int Draw(mglGraph *);
	void addLine(int size, double* x, double* y, double* z, const std::string& style_);
	void addLine(int size, double* x, double* y, double* z);
	void addSurf(int xSize, int ySize, double xMin, double xMax, double yMin, double yMax, double* data, const std::string& style_);
};


class QMGL1 : public QWidget {
	Q_OBJECT

public:
	QMGL1(QWidget *parent = 0);
	~QMGL1();
	void addLine(int size, double* x, double* y, double* z, const std::string& style_);
	void addLine(int size, double* x, double* y, double* z);
	void addSurf(int xSize, int ySize, double xMin, double xMax, double yMin, double yMax, double* data, const std::string& style_);
	void xLabel(const std::string& label);
	void yLabel(const std::string& label);
	void zLabel(const std::string& label);

protected:
	void ensurePolished();

private:
	QMathGL* mgl;
	AnotherDraw* draw;
	QSpinBox* phi;
	QSpinBox* teta;
	//QMenu* pMenu;
	QMenuBar* menu_bar;
	QFrame* tool_frame;
	QScrollArea* scroll;
	QBoxLayout* toolLayout;
	QTimer* resizeTimer;
	QTimer* linesAddTimer;
	bool squareAxis;

	void addMenu();
	void resizeEvent(QResizeEvent *event);

signals:    
	void squareChanged(bool);
	void gridChanged(bool);
	void boxChanged(bool);

private slots:    
	//void polish();
	void endOfResize();
	void linesAdded();
	void setBox(bool);
	void setGrid(bool);
	void setSquare(bool);
};

class QMGL2 : public QGLWidget {
	Q_OBJECT

public:
	QMGL2(QWidget *parent = 0);
	~QMGL2();

protected:

	void resizeGL(int nWidth, int nHeight);   // Method called after each window resize
	void paintGL();       // Method to display the image on the screen
	void initializeGL();  // Method to initialize OpenGL

private:
	//QMathGL* mgl;
	mglGraph* gr;
};