


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
	int id = qRegisterMetaType<SpectrogramInfo>();
	int id1 = qRegisterMetaType<CBPickerInfo>();
	int id2 = qRegisterMetaType<QWndPos>();
}
#else

QWorker::QWorker(): pf(nullptr) {

	/*hello();*/ 
	int id = qRegisterMetaType<SpectrogramInfo>();
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
int QWorker::mapview(int n) {
	int rv = 0;
	if (!QMetaObject::invokeMethod(this, "topviewImpl", Qt::BlockingQueuedConnection, Q_RETURN_ARG(int, rv), Q_ARG(int, n))) {
		xmprintf(0, "QWorker::mapview() cannot invoke topviewImpl\n");
	} else {
		xmprintf(6, "QWorker::mapview() OK; rv = %d\n", rv);
	}
	return rv;
}
#endif

#ifdef USEMATHGL
int QWorker::mglPlot(int n) {
	int rv = 0;
	if (!QMetaObject::invokeMethod(this, "mglPlotImpl", Qt::BlockingQueuedConnection,
			Q_RETURN_ARG(int, rv), Q_ARG(int, n))) {
		std::cout << " cannot invoke mglPlotImpl" << std::endl;
	}
	return rv;
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
	return (test == 0) ? 0 : test->iKey;	
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
void QWorker::spectrogram_info(const SpectrogramInfo& info) {
	int rv;
	xmprintf(4, "QWorker::spectrogram_info start \n");


	if (!QMetaObject::invokeMethod(this, "spectrogram_info_impl", Qt::BlockingQueuedConnection, Q_RETURN_ARG(int, rv),
		Q_ARG(SpectrogramInfo, info))) {

		xmprintf(1, "cannot invoke spectrogram_info \n");
	}
	xmprintf(4, "QWorker::spectrogram_info finished \n");
}

Q_INVOKABLE int  QWorker::spectrogram_info_impl(const SpectrogramInfo& info) {
	pf->setSpectrogramInfo(info);
	return 0;
}

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

int QWorker::qwtSavePng(int id, char* filename) {
	int rv = -5;
	if (!QMetaObject::invokeMethod(this, "qwtSavePngImpl", Qt::BlockingQueuedConnection, Q_RETURN_ARG(int, rv),
		Q_ARG(int, id), Q_ARG(char*, filename))) {

		std::cout << " cannot invoke QWorker::qwtSavePng" << std::endl;
	}
	return rv;
}

int QWorker::qwtSetPos(int key, QWndPos& pos) {
	int rv = -5;
	if (!QMetaObject::invokeMethod(this, "qwtSetPosImpl", Qt::BlockingQueuedConnection, Q_RETURN_ARG(int, rv),
			Q_ARG(int, key), Q_ARG(QWndPos&, pos))) {
		std::cout << " cannot invoke QWorker::qwtSavePng" << std::endl;
		xmprintf(2, "ERROR: qwtSetPosImpl invokeMethod failed \n");
	} else {
		xmprintf(2, "call to qwtSetPosImpl: ret = %d; x=%d; y = %d; w = %d; h = %d  \n", rv, pos.x, pos.y, pos.w, pos.h);
	}
	return rv;
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


void QWorker::qwtSetUdpCallback(OnUdpCallback cb) {
	if (!QMetaObject::invokeMethod(this, "qwtSetUdpCallbackImpl", Qt::BlockingQueuedConnection)) {
		std::cout << " cannot invoke qwtSetUdpCallbackImpl" << std::endl;
	}
}

void QWorker::qwtSetPickerCallback(OnPickerCallback cb) {
	xmprintf(8, "QWorker::qwtSetPickerCallback!\n");
	if (!QMetaObject::invokeMethod(this, "qwtSetPickerCallbackImpl", Qt::BlockingQueuedConnection, Q_ARG(OnPickerCallback, cb))) {
		xmprintf(8, "QWorker::qwtSetPickerCallback ERROR\n");
		std::cout << " cannot invoke qwtSetPickerCallbackImpl" << std::endl;

		qwtSetPickerCallbackImpl(cb); // %|
	}	else {
		xmprintf(8, "QWorker::qwtSetPickerCallback OK\n");
	}
}

int QWorker::qwtfigure(int n, unsigned int flags) {
	int rv = 0;
	if (!QMetaObject::invokeMethod(this, "qwtfigureImpl", Qt::BlockingQueuedConnection, 
			Q_RETURN_ARG(int, rv), Q_ARG(int, n), Q_ARG(unsigned int, flags))) {
		std::cout << " cannot invoke qwtfigureImpl" << std::endl;
	}
	return rv;
}
int QWorker::qwtSpectrogram(int n, unsigned int flags) {
	int rv = 0;
	if (!QMetaObject::invokeMethod(this, "qwtSpectrogramImpl", Qt::BlockingQueuedConnection,
		Q_RETURN_ARG(int, rv), Q_ARG(int, n), Q_ARG(unsigned int, flags))) {
		std::cout << " cannot invoke qwtSpectrogramImpl" << std::endl;
	}
	return rv;

}

int QWorker::qwtservice(int x) {
	int rv = 0;
	if (!QMetaObject::invokeMethod(this, "qwtserviceImpl", Qt::BlockingQueuedConnection, 
			Q_RETURN_ARG(int, rv), Q_ARG(int, x))) {
		std::cout << " cannot invoke qwtserviceImpl" << std::endl;
	}
	return rv;
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
Q_INVOKABLE int QWorker::qwtSavePngImpl(int id, char* filename) {
	int test = pf->savePng(id, filename);
	return test;
}

Q_INVOKABLE int QWorker::qwtSetPosImpl(int key, QWndPos& pos) {
	int test = pf->setPos(key, pos);
	return test;
}

Q_INVOKABLE int QWorker::qwtfigureImpl(int n, unsigned int flags) {
	JustAplot*  test = pf->figure(n, jQWT, flags);
	return (test == 0) ? 0 : test->iKey;
}

Q_INVOKABLE int QWorker::qwtSpectrogramImpl(int n, unsigned int flags) {
	JustAplot* test = pf->figure(n, jQwSpectrogram, flags);
	return (test == 0) ? 0 : test->iKey;
}

Q_INVOKABLE int QWorker::qwtserviceImpl(int x) {
	int test = pf->service(x);
	return test;
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

Q_INVOKABLE void QWorker::qwtSetUdpCallbackImpl(OnUdpCallback cb) {
	if (pf != 0) {
		pf->setUdpCallback(cb);
	}
}

Q_INVOKABLE void QWorker::qwtSetPickerCallbackImpl(OnPickerCallback cb) {
	xmprintf(8, "QWorker::qwtSetPickerCallbackImpl! \n");
	if (pf != 0) {
		xmprintf(8, "QWorker::qwtSetPickerCallbackImpl!!!! \n");
		pf->setPickerCallback(cb);
	}
}

Q_INVOKABLE int QWorker::qtstartImpl() {
	int test = qwSettings.qwLoad();
	int test1 = qwSettings.qwSave();
	xmprintf(7, "starting QWorker::qtstartImpl()\n");
	if (pf == nullptr) {
		xmprintf(3, "PATH inside qtstartImpl: %s\n\n", std::getenv("PATH"));
		pf = new XQPlots();
		xmprintf(5, "\tXQPlots created\n ");
		if (pf == nullptr) { //  fail
			std::cout << " error qtstartImpl: (pf == nullptr)" << std::endl;
		}	else {
			//std::cout << "q2  started! " << std::endl;
				QIcon icon;
    			icon.addPixmap(QPixmap(QString::fromUtf8(":/icons/binokl.png")), QIcon::Normal, QIcon::Off);
				icon.addPixmap(QPixmap(QString::fromUtf8(":/icons/binokl.png")), QIcon::Normal, QIcon::On);
			pf->setWindowIcon(icon);

			//   those MarbleDirs were done already in 'main()'; why we need this again from inside QT gui thread?

			//#ifdef USEMARBLE
			//Marble::MarbleDirs::setMarbleDataPath(QString::fromStdString(mdPath));
			//Marble::MarbleDirs::setMarblePluginPath(QString::fromStdString(mpPath));
			//#endif			
		}
	}	else {
		xmprintf(5, "\talready started .. \n");
	}
	xmprintf(7, "QWorker::qtstartImpl() ended\n");
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
	xmprintf(7, "QWorker::topviewImpl(): test = %d \n", (test == 0) ? 0 : test->iKey);
	return (test == 0) ? 0 : test->iKey;
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



