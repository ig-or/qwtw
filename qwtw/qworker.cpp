


#include "xstdef.h"
#include "xmutils.h"
#include "sfigure.h"
#include "justaplot.h"
#include "qwproc.h"

#include "qworker.h"
#include "qwtypes.h"
#include "settings.h"

#include <iostream>
#ifdef WIN32
#include "windows.h"
#endif
#ifdef USEMARBLE
#include <marble/MarbleDirs.h>
#endif

#ifdef USEMARBLE
QWorker::QWorker(const std::string& mdp, const std::string& mpp): pf(nullptr) {
	mdPath = mdp;
	mpPath = mpp;
}
#else

QWorker::QWorker(): pf(nullptr) {

	/*hello();*/ 
}
#endif
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
void QWorker::mapview(int n) {
	int rv;
	QMetaObject::invokeMethod(this, "topviewImpl", Qt::BlockingQueuedConnection, Q_RETURN_ARG(int, rv), Q_ARG(int, n));
}
#endif

#ifdef USEMATHGL
void QWorker::mglPlot(int n) {
	int rv;
	QMetaObject::invokeMethod(this, "mglPlotImpl", Qt::BlockingQueuedConnection, 
		Q_RETURN_ARG(int, rv), Q_ARG(int, n));
}

void QWorker::mgl_line(int size, double* x, double* y, double* z, const char* name, const char* style) {
	int rv;
	if (!QMetaObject::invokeMethod(this, "mgl_lineImpl", Qt::BlockingQueuedConnection, Q_RETURN_ARG(int, rv),
		Q_ARG(int, size),
		Q_ARG(double*, x), Q_ARG(double*, y), Q_ARG(double*, z), 
		Q_ARG(const char*, name), Q_ARG(const char*, style)
		)) {

		std::cout << " cannot invoke mgl_lineImpl" << std::endl;
	}
}

void QWorker::mgl_mesh(int xSize, int ySize, 
		double xMin, double xMax, double yMin, double yMax, 
		double* data, const char* name,
		const char* style,
		int type) {

	int rv;
	xmprintf(4, "QWorker::mgl_mesh start \n");
	if (!QMetaObject::invokeMethod(this, "mgl_meshImpl", Qt::BlockingQueuedConnection, Q_RETURN_ARG(int, rv),
		Q_ARG(int, xSize), Q_ARG(int, ySize),
		Q_ARG(double, xMin),  Q_ARG(double, xMax), Q_ARG(double, yMin),  Q_ARG(double, yMax), 
		Q_ARG(double*, data),
		Q_ARG(const char*, name), Q_ARG(const char*, style),
		Q_ARG(int, type))) 		{

		std::cout << " cannot invoke mgl_meshImpl" << std::endl;
	}
	xmprintf(4, "QWorker::mgl_mesh finished \n");
	
}

Q_INVOKABLE int QWorker::mglPlotImpl(int n) {
	JustAplot* test = pf->figure(n, jMathGL);
	return (test == 0) ? 1 : 0;	
	return 0;
}

Q_INVOKABLE int QWorker::mgl_lineImpl(int size, double* x, double* y, double* z, const char* name, const char* style) {
	pf->setmode(3);
	xmprintf(6, "\tmgl_lineImpl. size = %d \n", size);
	pf->plot(x, y, z, size, name, style, 1, 1, 0);
	return 0;	
}

Q_INVOKABLE int QWorker::mgl_meshImpl(int xSize, int ySize, 
		double xMin, double xMax, double yMin, double yMax, 
		double* data, const char* name,
		const char* style,
		int type) {

	SurfDataType sd = static_cast<SurfDataType>(type);
	xmprintf(6, "QWorker::mgl_meshImpl; xSize = %d; ySize = %d\n", xSize, ySize);
	pf->mesh(MeshInfo{xSize, ySize, xMin, xMax, yMin, yMax, data, name, style, sd});
	
	return 0;
}
#endif

int QWorker::qVersion(char* vstr, int vstr_size) {
#ifdef WIN32
#ifdef qwtwcEXPORTS
	extern HMODULE qwtwLibModule;
	return xqversion(vstr, vstr_size - 2, qwtwLibModule);
#else
	return xqversion(vstr, vstr_size - 2);
#endif
	
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
void QWorker::qwtsetimpstatus(int impStatus) {
	//int rv;
	if (!QMetaObject::invokeMethod(this, "qwtsetimpstatusImpl", Qt::QueuedConnection, Q_ARG(int, impStatus))) {
		std::cout << " cannot invoke qwtsetimpstatusImpl" << std::endl;
	}
}

void QWorker::qwtxlabel(const char* s) {
	//int rv;
	if (!QMetaObject::invokeMethod(this, "qwtxlabelImpl", Qt::BlockingQueuedConnection, Q_ARG(const char*, s))) {
		std::cout << " cannot invoke qwtxlabelImpl" << std::endl;
	}
}

void QWorker::qwtylabel(const char* s) {
	//int rv;
	if (!QMetaObject::invokeMethod(this, "qwtylabelImpl", Qt::BlockingQueuedConnection,  Q_ARG(const char*, s))) {
		std::cout << " cannot invoke qwtylabelImpl" << std::endl;
	}
}



int QWorker::qwtplot2(double* x, double* y, int size, const char* name, const char* style, int lineWidth, int symSize, double* time) {
	int rv = -5;
	if (!QMetaObject::invokeMethod(this, "qwtplot2Impl", Qt::BlockingQueuedConnection, Q_RETURN_ARG(int, rv),
		Q_ARG(double*, x), Q_ARG(double*, y), Q_ARG(int, size), Q_ARG(const char*, name), Q_ARG(const char*, style),
		Q_ARG(int, lineWidth), Q_ARG(int, symSize), Q_ARG(double*, time))) {

		std::cout << " cannot invoke qwtplot2Impl" << std::endl;
	}
	return rv;
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

void QWorker::qwtSetClipGroup(int gr) {
	if (!QMetaObject::invokeMethod(this, "qwtSetClipGroupImpl", Qt::QueuedConnection, Q_ARG(int, gr))) {
		std::cout << " cannot invoke qwtSetClipGroupImpl" << std::endl;
	}
}

void QWorker::qwtRemoveLine(int key) {
	if (!QMetaObject::invokeMethod(this, "qwtRemoveLineImpl", Qt::QueuedConnection, Q_ARG(int, key))) {
		std::cout << " cannot invoke qwtRemoveLineImpl" << std::endl;
	}
}

int QWorker::qwtChangeLine(int id, double* x, double* y, double* z, double* time, int size) {
	int rv = -5;
	if (!QMetaObject::invokeMethod(this, "qwtChangeLineImpl", Qt::BlockingQueuedConnection, Q_RETURN_ARG(int, rv),
		Q_ARG(int, id),
		Q_ARG(double*, x), Q_ARG(double*, y), Q_ARG(double*, z), Q_ARG(double*, time),
		Q_ARG(int, size))) {

		std::cout << " cannot invoke qwtChangeLineImpl" << std::endl;
	}
	return rv;
}

int QWorker::qwtplot(double* x, double* y, int size, const char* name, const char* style, int lineWidth, int symSize) {
	int rv = -5;
	if (!QMetaObject::invokeMethod(this, "qwtplotImpl", Qt::BlockingQueuedConnection, Q_RETURN_ARG(int, rv),
		Q_ARG(double*, x), Q_ARG(double*, y), Q_ARG(int, size), Q_ARG(const char*, name), Q_ARG(const char*, style),
		Q_ARG(int, lineWidth), Q_ARG(int, symSize))) {

		std::cout << " cannot invoke qwtplotImpl" << std::endl;
	}
	return rv;
}


Q_INVOKABLE int QWorker::qwttitleImpl(const char* s) {
	pf->title(s);
	return 24;
}

Q_INVOKABLE int QWorker::qwtplot2Impl(double* x, double* y, int size, const char* name, 
		const char* style, int lineWidth, int symSize, double* time) {
	pf->setmode(3);
	int test = pf->plot(x, y, size, name, style, lineWidth, symSize, time);
	return test;
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

Q_INVOKABLE void QWorker::qwtsetimpstatusImpl(int impStatus) {
	pf->setImportant(impStatus != 0);
}

Q_INVOKABLE void QWorker::qwtxlabelImpl(const char* s) {
	pf->xlabel(s);
}
Q_INVOKABLE void QWorker::qwtylabelImpl(const char* s) {
	pf->ylabel(s);
}

Q_INVOKABLE int QWorker::qwtfigureImpl(int n) {
	JustAplot*  test = pf->figure(n, jQWT);
	return (test == 0) ? 1 : 0;
}

Q_INVOKABLE void QWorker::qwtSetClipGroupImpl(int gr) {
	pf->setClipGroup(gr);
}

Q_INVOKABLE void QWorker::qwtRemoveLineImpl(int key) {
	pf->removeLine(key);
}

Q_INVOKABLE int  QWorker::qwtChangeLineImpl(int id, double* x, double* y, double* z, double* time, int size) {
	int test = pf->changeLine(id, x, y, z, time, size);
	return test;
}

Q_INVOKABLE int QWorker::qwtplotImpl(double* x, double* y, int size, const char* name, const char* style, int lineWidth, int symSize) {
	int test = pf->plot(x, y, size, name, style, lineWidth, symSize);
	return test;
}



Q_INVOKABLE int QWorker::qtstartImpl() {
	int test = qwSettings.qwLoad();
	
	if (pf == nullptr) {
		xmprintf(3, "PATH inside qtstartImpl: %s\n\n", std::getenv("PATH"));
		pf = new XQPlots();
		if (pf == nullptr) { //  fail
			std::cout << " error qtstartImpl: (pf == nullptr)" << std::endl;
		}	else {
			//std::cout << "q2  started! " << std::endl;
				QIcon icon;
    			icon.addPixmap(QPixmap(QString::fromUtf8(":/icons/binokl.png")), QIcon::Normal, QIcon::Off);
				icon.addPixmap(QPixmap(QString::fromUtf8(":/icons/binokl.png")), QIcon::Normal, QIcon::On);
			pf->setWindowIcon(icon);

			#ifdef USEMARBLE
			Marble::MarbleDirs::setMarbleDataPath(mdPath.c_str());
			Marble::MarbleDirs::setMarblePluginPath(mpPath.c_str());
			#endif			
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
	JustAplot* test = pf->figure(n, jMarble);
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



