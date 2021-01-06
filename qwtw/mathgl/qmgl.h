#pragma once

//#include <mgl2/qmathgl.h>

//
//#include <QFrame>
#include <QGLWidget>

//class QMathGL;
class mglGraph;

class QMGL : public QGLWidget {
    Q_OBJECT

public:
    QMGL(QWidget *parent = 0);
    ~QMGL();

protected:

    void resizeGL(int nWidth, int nHeight);   // Method called after each window resize
    void paintGL();       // Method to display the image on the screen
    void initializeGL();  // Method to initialize OpenGL

private:
    //QMathGL* mgl;
    mglGraph* gr;
};