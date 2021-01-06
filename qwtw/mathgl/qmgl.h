#pragma once

//#include <mgl2/qmathgl.h>

//
//#include <QFrame>
#include <QWidget>
#include <QGLWidget>

class QMathGL;
class mglGraph;

class QMGL1 : public QWidget {
    Q_OBJECT

public:
    QMGL1(QWidget *parent = 0);
    ~QMGL1();

protected:

private:
    QMathGL* mgl;
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