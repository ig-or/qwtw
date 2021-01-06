
//#include <QLayout>
#include <QtOpenGL>


#include <mgl2/qmathgl.h>
#include "qmgl.h"


int sample(mglGraph *gr)
{
  gr->Rotate(60,40);
  gr->Box();
  return 0;
}


QMGL1::QMGL1(QWidget *parent) : QWidget(parent) {
    mgl = new QMathGL(this);
    mgl->setDraw(sample);

    mgl->setZoom(true);
    mgl->setRotate(true);

    mgl->update();
}
QMGL1::~QMGL1() {
    if (mgl) {
        delete mgl;
    }
}


QMGL2::QMGL2(QWidget *parent) : QGLWidget(parent), gr(0)  {

}
QMGL2::~QMGL2() {
    if(gr)	{
        delete gr;
    }
}

void QMGL2::initializeGL()	{// recreate instance of MathGL core
	if(gr)	delete gr;
	gr = new mglGraph(1);	// use '1' for argument to force OpenGL output in MathGL
}

void QMGL2::resizeGL(int w, int h) {// standard resize replace
	QGLWidget::resizeGL(w, h);
	glViewport (0, 0, w, h);
}

void QMGL2::paintGL()  {	// main drawing function
	gr->Clf();	// clear previous OpenGL primitives
	gr->SubPlot(1,1,0);
	gr->Rotate(40,60);
	gr->Light(true);
	gr->AddLight(0,mglPoint(0,0,10),mglPoint(0,0,-1));
	gr->Axis();
	gr->Box();
	gr->FPlot("sin(pi*x)","i2");
	gr->FPlot("cos(pi*x)","|");
	gr->FSurf("cos(2*pi*(x^2+y^2))");
	gr->Finish();
	swapBuffers();	// show output on the screen
}

