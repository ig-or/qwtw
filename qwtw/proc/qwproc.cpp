

#include "qwproc.h"
#include "xstdef.h"
#include "qworker.h"
#include "qwproto.h"

#include <QObject>
#include <QPointer>
#include <QApplication>




static QPointer<QWorker> q2worker(nullptr);

int main(int argc, char** argv) {

    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);   
    QWorker qWorker;
    qWorker.qtstart(false);
    QProcInterface qpi;
    qpi.start();
    QObject::connect(&app, SIGNAL(aboutToQuit()), &qWorker, SLOT(onQtAppClosing()));
    
    app.exec();
    
    return 0;
}


void assert_failed(const char* file, unsigned int line, const char* str) {
	xm_printf("ASSERT faild: %s (file %s, line %d)\n", str, file, line);
}


