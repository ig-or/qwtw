

#include "qwproc.h"
#include "xstdef.h"
#include "qworker.h"
#include "qwproto.h"

#include <QObject>
#include <QPointer>
#include <QApplication>


static QPointer<QWorker> q2worker(nullptr);

int main(int argc, char** argv) {
    setbuf(stdout, NULL);
	setvbuf(stdout, NULL, _IONBF, 0);
    printf("main: starting \n");

    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);   
    QWorker qWorker;
    qWorker.qtstart(false);
    QProcInterface qpi(qWorker, app);
    qpi.start();
    QObject::connect(&app, SIGNAL(aboutToQuit()), &qWorker, SLOT(onQtAppClosing()));
    //qWorker.appMutex.lock();
    printf("main: starting QT thread  \n");
    app.exec();
    printf("main: QT thread finished  \n");
    //qWorker.appMutex.unlock();
    //qWorker.appV.notify_all();
    
    printf("main: exiting \n");
    return 0;
}


void assert_failed(const char* file, unsigned int line, const char* str) {
	xm_printf("ASSERT faild: %s (file %s, line %d)\n", str, file, line);
}


