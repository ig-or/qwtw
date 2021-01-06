#pragma once

//#include <mgl2/qmathgl.h>

#include <QLayout>
#include <QFrame>

class QMathGL;

class QMGL : public QWidget {
    Q_OBJECT

    public:
    QMGL(QWidget *parent = 0);

    private:
    QMathGL* mgl;

};