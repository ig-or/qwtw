


#include "xstdef.h"
#include "sfigure.h"


#include "qworker.h"

#include <iostream>


QWorker::QWorker(): pf(nullptr) {
	/*hello();*/ 
}

void QWorker::onQtAppClosing() {
	if (pf != nullptr) {
		xm_printf("QWorker::onQtAppClosing()! \n \tdeleting 'pf'... \n");
		//delete pf;  


		//pf->onExit();

		//printf("OK!\n");
		//pf = nullptr;
		//std::cout << "QWorker::onQtAppClosing() ends " << std::endl;
	}
}

QWorker::~QWorker() {
	if (pf != nullptr) {
		xm_printf("QWorker::~QWorker() \n");
		//delete pf;  // this is not 100% OK here, not a QT thread
		//pf = nullptr;
		//std::cout << "QWorker::~QWorker(): pf was deleted!  this is not 100% OK here, not a QT thread" << std::endl;
	}
}

void QWorker::hello() {
	// This is thread-safe, the method is invoked from the event loop
	if (!QMetaObject::invokeMethod(this, "helloImpl", Qt::QueuedConnection)) {
		std::cout << " cannot invoke helloImpl" << std::endl;
	}
}
void QWorker::qwtshowmw() {
	if (!QMetaObject::invokeMethod(this, "qwtshowmwImpl", Qt::QueuedConnection)) {
		std::cout << " cannot invoke qwtshowmwImpl" << std::endl;
	}
}
#ifdef USEMARBLE
void QWorker::topview(int n) {
	int rv;
	QMetaObject::invokeMethod(this, "topviewImpl", Qt::BlockingQueuedConnection, Q_RETURN_ARG(int, rv), Q_ARG(int, n));
}
#endif
int QWorker::qVersion(char* vstr, int vstr_size) {
#ifdef WIN32
	return xqversion(vstr, vstr_size - 2, qwtwLibModule);
#else
	strcpy(vstr, " linux? ");
	return 7;
#endif
}
int QWorker::qwttitle(const char* s) {  //  blocking because have to copy param "s"
	int rv;
	//QString s1 = s;
	if (!QMetaObject::invokeMethod(this, "qwttitleImpl", Qt::BlockingQueuedConnection, Q_RETURN_ARG(int, rv), Q_ARG(const char*, s))) {
		std::cout << " cannot invoke qwttitleImpl" << std::endl;
	}
	return rv;
}
void QWorker::qwtEnableCoordBroadcast(double* x, double* y, double* z, double* time, int size) {
	if (!QMetaObject::invokeMethod(this, "qwtEnableCoordBroadcastImpl", Qt::BlockingQueuedConnection, 
		Q_ARG(double*, x), Q_ARG(double*, y), Q_ARG(double*, z),
		Q_ARG(double*, time), Q_ARG(int, size))) {
		std::cout << " cannot invoke qwtEnableCoordBroadcastImpl" << std::endl;
	}

}
void QWorker::qwtDisableCoordBroadcast() {
	if (!QMetaObject::invokeMethod(this, "qwtDisableCoordBroadcastImpl", Qt::QueuedConnection)) {
		std::cout << " cannot invoke qwtDisableCoordBroadcastImpl" << std::endl;
	}
}
void QWorker::qwtsetimpstatus(int status) {
	//int rv;
	if (!QMetaObject::invokeMethod(this, "qwtsetimpstatusImpl", Qt::QueuedConnection, Q_ARG(int, status))) {
		std::cout << " cannot invoke qwtsetimpstatusImpl" << std::endl;
	}
}

void QWorker::qwtxlabel(const char* s) {
	int rv;
	if (!QMetaObject::invokeMethod(this, "qwtxlabelImpl", Qt::BlockingQueuedConnection, Q_ARG(const char*, s))) {
		std::cout << " cannot invoke qwtxlabelImpl" << std::endl;
	}
}

void QWorker::qwtylabel(const char* s) {
	int rv;
	if (!QMetaObject::invokeMethod(this, "qwtylabelImpl", Qt::BlockingQueuedConnection,  Q_ARG(const char*, s))) {
		std::cout << " cannot invoke qwtylabelImpl" << std::endl;
	}
}



void QWorker::qwtplot2(double* x, double* y, int size, const char* name, const char* style, int lineWidth, int symSize, double* time) {
	int rv;
	if (!QMetaObject::invokeMethod(this, "qwtplot2Impl", Qt::BlockingQueuedConnection, Q_RETURN_ARG(int, rv),
		Q_ARG(double*, x), Q_ARG(double*, y), Q_ARG(int, size), Q_ARG(const char*, name), Q_ARG(const char*, style),
		Q_ARG(int, lineWidth), Q_ARG(int, symSize), Q_ARG(double*, time))) {

		std::cout << " cannot invoke qwtplot2Impl" << std::endl;
	}
}

void QWorker::qwtclear() {
	if (!QMetaObject::invokeMethod(this, "qwtclearImpl", Qt::QueuedConnection)) {
		std::cout << " cannot invoke qwtclearImpl" << std::endl;
	}
}

void QWorker::qwtfigure(int n) {
//	int rv;
	if (!QMetaObject::invokeMethod(this, "qwtfigureImpl", Qt::QueuedConnection,  Q_ARG(int, n))) {
		std::cout << " cannot invoke qwtfigureImpl" << std::endl;
	}
}

void QWorker::qwtplot(double* x, double* y, int size, const char* name, const char* style, int lineWidth, int symSize) {
	int rv;
	if (!QMetaObject::invokeMethod(this, "qwtplotImpl", Qt::BlockingQueuedConnection, 
		Q_ARG(double*, x), Q_ARG(double*, y), Q_ARG(int, size), Q_ARG(const char*, name), Q_ARG(const char*, style),
		Q_ARG(int, lineWidth), Q_ARG(int, symSize))) {

		std::cout << " cannot invoke qwtplotImpl" << std::endl;
	}
}


Q_INVOKABLE int QWorker::qwttitleImpl(const char* s) {
	pf->title(s);
	return 24;
}

Q_INVOKABLE int QWorker::qwtplot2Impl(double* x, double* y, int size, const char* name, 
		const char* style, int lineWidth, int symSize, double* time) {
	pf->setmode(3);
	pf->plot(x, y, size, name, style, lineWidth, symSize, time);
	return 0;
}
Q_INVOKABLE void QWorker::qwtEnableCoordBroadcastImpl(double* x, double* y, double* z, double* time, int size) {
	pf->enableCoordBroadcast(x, y, z, time, size);
}
Q_INVOKABLE void QWorker::qwtDisableCoordBroadcastImpl() {
	pf->disableCoordBroacast();
}

Q_INVOKABLE void QWorker::qwtclearImpl() {
	pf->clear();
}

Q_INVOKABLE void QWorker::qwtsetimpstatusImpl(int status) {
	pf->setImportant(status != 0);
}

Q_INVOKABLE void QWorker::qwtxlabelImpl(const char* s) {
	pf->xlabel(s);
}
Q_INVOKABLE void QWorker::qwtylabelImpl(const char* s) {
	pf->ylabel(s);
}

Q_INVOKABLE int QWorker::qwtfigureImpl(int n) {
	JustAplot*  test = pf->figure(n, 1);
	return (test == 0) ? 1 : 0;
}
Q_INVOKABLE void QWorker::qwtplotImpl(double* x, double* y, int size, const char* name, const char* style, int lineWidth, int symSize) {
	pf->plot(x, y, size, name, style, lineWidth, symSize);
}



Q_INVOKABLE int QWorker::qtstartImpl() {
	if (pf == nullptr) {
		//xm_printf("PATH inside qtstartImpl: %s\n\n", std::getenv("PATH"));
		pf = new XQPlots();
		if (pf == nullptr) { //  fail
			std::cout << " error qtstartImpl: (pf == nullptr)" << std::endl;
		}	else {
			//std::cout << "q2  started! " << std::endl;
		}
	} 
	return 25;
}
Q_INVOKABLE void QWorker::qwtshowmwImpl() {
	if (pf != nullptr) {
		pf->showMainWindow();
	}
}
#ifdef USEMARBLE
Q_INVOKABLE int QWorker::topviewImpl(int n) {
	JustAplot* test = pf->figure(n, 2);
	return (test == 0) ? 1 : 0;
}
#endif

int QWorker::qtstart(bool wait) {
	int rv;
	//QMetaObject::invokeMethod(this, "qtstartImpl", Qt::BlockingQueuedConnection, Q_RETURN_ARG(int, rv));
	Qt::ConnectionType ct = wait ? Qt::BlockingQueuedConnection : Qt::QueuedConnection;
	if (wait) {
		QMetaObject::invokeMethod(this, "qtstartImpl", Qt::BlockingQueuedConnection, Q_RETURN_ARG(int, rv));
	}	else {
		QMetaObject::invokeMethod(this, "qtstartImpl", Qt::QueuedConnection);
	}
	return wait ? rv : 0;
	//return 0;
}



