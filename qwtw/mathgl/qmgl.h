#pragma once

//#include <mgl2/qmathgl.h>
#include <mgl2/wnd.h>
#include <mgl2/canvas.h>
#include <mgl2/qmathgl.h>


//
//#include <QFrame>
#include <QWidget>
#include <QGLWidget>

class QMathGL;
class mglGraph;
class QSpinBox;
class QFrame;
class QMenu;
class QTimer;
class QMenuBar;
class QBoxLayout;
class QResizeEvent;

class OurMathGL : public QMathGL {
    public:
	OurMathGL(QWidget *parent = 0, Qt::WindowFlags f = 0);
	virtual ~OurMathGL();
};

class AnotherDraw : public mglDraw {
    int linesCount;
    double xMin, xMax, yMin, yMax, zMin, zMax;
    //mglGraph *gr;

   	mglData mx;
	mglData my;
	mglData mz;
    int drawCounter;
    int endOfResizeFlag;
    void updateRange(int size, double* x, double* y, double* z);

public:
    void onResize();
    AnotherDraw();
    ~AnotherDraw();
    int Draw(mglGraph *);
    void addLine(int size, double* x, double* y, double* z);
};


class QMGL1 : public QWidget {
    Q_OBJECT

public:
    QMGL1(QWidget *parent = 0);
    ~QMGL1();
    void addLine(int size, double* x, double* y, double* z);

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
    QBoxLayout* toolLayout;
    QTimer* resizeTimer;


    


    void addMenu();
    void resizeEvent(QResizeEvent *event);

private slots:    
    void polish();
    void endOfResize();
    
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