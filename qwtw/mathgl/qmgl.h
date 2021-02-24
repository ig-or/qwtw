#pragma once

//#include <mgl2/qmathgl.h>
#include <mgl2/wnd.h>
#include <mgl2/canvas.h>
#include <mgl2/qmathgl.h>
#include "qwtypes.h"


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

struct LineItemInfo;


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
	std::string legend;
	//ThreeDline(int size, double* x, double* y, double* z, const std::string& style_);
	ThreeDline(LineItemInfo* line);
};

struct SurfData {

	mglData f;
	ARange range;
	std::string style;
	SurfDataType sdType;

	/** 
	 * \param data double array [xSize x ySize]; point M(x[i], y[j]) have data[i + xSize*j]
	 * */
	SurfData(const MeshInfo& info);
};

class AnotherDraw : public mglDraw {
	ARange range;
	//mglGraph *gr;
	//std::list< std::shared_ptr<ThreeDline>> lines;
	std::list<ThreeDline> lines;
	std::list<std::shared_ptr<SurfData>> surfs;

public:

	int drawCounter;
	int endOfResizeFlag;
	int plotsCount;
	
	std::string xLabel, yLabel, zLabel;
	//std::string title;
	bool useBox, useGrid;
	//SurfDataType sdType;

	void onResize();
	AnotherDraw();
	~AnotherDraw();
	int Draw(mglGraph *);
	//void addLine(int size, double* x, double* y, double* z, const std::string& style_);
	//void addLine(int size, double* x, double* y, double* z);
	void addLine(LineItemInfo* ii);
	void addSurf(const MeshInfo& info);
};


class QMGL1 : public QWidget {
	Q_OBJECT

public:
	QMGL1(QWidget *parent = 0);
	~QMGL1();
	//void addLine(int size, double* x, double* y, double* z, const std::string& style_);
	void addLine(LineItemInfo* line);
	//void addLine(int size, double* x, double* y, double* z);
	void addSurf(const MeshInfo& info);
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
	bool dotsPreview;

	void addMenu();
	void resizeEvent(QResizeEvent *event);

signals:    
	void squareChanged(bool);
	void gridChanged(bool);
	void boxChanged(bool);
	void dotsPreviewChanged(bool);

private slots:    
	//void polish();
	void endOfResize();
	void linesAdded();
	void setBox(bool);
	void setGrid(bool);
	void setSquare(bool);
	void setDotsPreview(bool);
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
