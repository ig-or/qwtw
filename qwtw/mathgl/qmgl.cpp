
#include <mgl2/qmathgl.h>
#include "qmgl.h"

int sample(mglGraph *gr)
{
  gr->Rotate(60,40);
  gr->Box();
  return 0;
}

QMGL::QMGL(QWidget *parent) : QWidget(parent)  {
    mgl = new QMathGL(this);
    mgl->setDraw(sample);
    mgl->update();
}