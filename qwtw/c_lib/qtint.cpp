
#include "qwtw.h"
#include "qtint.h"
#include "sfigure.h"
#include "xqbytebuffer.h"
#include "xstdef.h"
//#include "qtswidget.h"
#include "xmutils.h"
#include "sfigure.h"
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <QApplication>
#include <thread>
#include <chrono>
#include <cstdlib>

#include <mutex>
#include <condition_variable>
#include <locale>
#include <codecvt>
#include <string>


// ################################################################
// ################################################################
// ################################################################
// ################################################################
#ifdef WIN32
#include <Windows.h>
HMODULE qwtwLibModule = 0;
#endif
#include <stdlib.h>



#ifndef MAX_PATH  //  linux?
#define MAX_PATH 260
#endif


//QWController* qwtController = 0;
//QWTest* qwTest = 0;
QApplication* qApp123 = 0;
boost::thread* bt = 0;

static int trCounter = 0, prCounter = 0;
//LRBuf<BQInfo, 16>* bq = 0;

/*
QWController::QWController() {
	ok = false;
	
}

int QWController::init(LRBuf<BQInfo, 16>* bq_) {
	bq = bq_;
	return 0;
}

#ifdef LIN_UX
	#include <signal.h>
#endif

void QWController::close() {
    //xm_printf("got QWController::close() \n");
    try {
	   send(0, 0, 0);  //    exit eberybody!
    } catch(...) {
	   xm_printf("QWController::close(): send(0, 0, 0) faild \n");
    }
    boost::this_thread::sleep(boost::posix_time::milliseconds(54));
	//bq->abort();
    boost::this_thread::sleep(boost::posix_time::milliseconds(10));
	if (bt != 0) {
        bt->try_join_for(boost::chrono::milliseconds(24));
		bt->interrupt();
#ifdef WIN32
		TerminateThread(bt->native_handle(), 0);
#else
        bt->try_join_for(boost::chrono::milliseconds(10));
    //	pthread_kill(bt->native_handle(), 9);
#endif

    }
}



void QWController::send(unsigned char cmd, const char* b, int size, bool waitForReply) {
	BQInfo i;
	i.cmd = cmd;
	//strncpy(i.data, b, BQInfo::dsize - 1);
	//qmw->command(i);
	int bs = size;
	mxassert(size <= BQInfo::dsize, "");
	if (size >= BQInfo::dsize) {
		xm_printf("ERROR: QWController::send overflow\n");
		bs = BQInfo::dsize - 1;
	}
	if (bs > 0) {
		memcpy(i.data, b, bs);
	}
	bq->put(i, waitForReply);
	//bq->put(i);
}

void QWController::figure(int n) {
	char m[64]; XQByteBuffer b(m, false);
	b.putInt(n);
	send(1, m, b.getSize());
}

#ifdef USEMARBLE
void QWController::figure_topview(int n) {
	char m[64]; XQByteBuffer b(m, false);
	b.putInt(n);
	send(7, m, b.getSize());
}
#endif

#ifdef USE_QT3D
void QWController::figure_3d(int n) {
	char m[64]; XQByteBuffer b(m, false);
	b.putInt(n);
	send(8, m, b.getSize());
}
#endif

void QWController::title(char* s) {
	char m[256]; XQByteBuffer b(m, 255);
	b.putString(s); 
	send(2, m, b.getSize());
}

void QWController::xlabel(char* s) {
	char m[256]; XQByteBuffer b(m, 255);
	b.putString(s); 
	send(3, m, b.getSize());
}

void QWController::ylabel(char* s) {
	char m[256]; XQByteBuffer b(m, 255);
	b.putString(s); 
	send(4, m, b.getSize());
}

void QWController::setmode(int mode) {
    char m[64]; XQByteBuffer b(m, false);
    b.putInt(mode);
    send(6, m, b.getSize());
}

void QWController::clear() {
	send(11, 0, 0);
}

void QWController::showMW() {
	send(12, 0, 0);
}


void QWController::setImportantStatus(int status) {
	char m[64]; XQByteBuffer b(m, false);
	b.putInt(status);
	send(10, m, b.getSize());
}

void QWController::plot(double* x, double* y, int size, char* name, const char* style, 
		  int lineWidth, int symSize, double* time) {
	char m[256];  int k = 0;
	XQByteBuffer b(m, false);
	b.putDPtr(x); b.putDPtr(y); b.putInt(size);
	b.putString(name);  b.putString(style);  b.putInt(lineWidth); b.putInt(symSize);
	b.putDPtr(time);
	send(5, m, b.getSize(), true);
}
void QWController::plot(double* x, double* y, double* z, int size, char* name, const char* style,
	int lineWidth, int symSize, double* time) {
	char m[256];  int k = 0;
	XQByteBuffer b(m, false);
	b.putDPtr(x); b.putDPtr(y);  b.putDPtr(z); b.putInt(size);
	b.putString(name);  b.putString(style);  b.putInt(lineWidth); b.putInt(symSize);
	b.putDPtr(time);
	send(20, m, b.getSize(), true);
}

#ifdef ENABLE_UDP_SYNC
void QWController::qwtEnableCoordBroadcast(double* x, double* y, double* z, double* time, int size) { // 9
	char m[256];  int k = 0;
	XQByteBuffer b(m, false);
	b.putDPtr(x); b.putDPtr(y);  b.putDPtr(z); b.putDPtr(time); b.putInt(size);
	send(9, m, b.getSize(), true);
}
void QWController::qwtDisableCoordBroadcast() {
	char m[32];  int k = 0;
	send(90, m, 0, true);
}
#endif
*/

// ################################################################
// ################################################################
// ################################################################
// ################################################################



// ################################################################
// ################################################################
// ################################################################
// ################################################################

/*
QWTest::QWTest(LRBuf<BQInfo, 16>* bq_): QTSMainWidget(bq_) {
	pf = 0;
}

void QWTest::onInfo(BQInfo x) {
    //xm_printf("QWTest::onInfo() !!!!!!!!!!! cmd = %d\n", x.cmd);
	if (pf == 0) {
		//pf = new XQPlots(this);
		pf = new XQPlots();
	}
	int n, mode;
	XQByteBuffer b(x.data, false);
	char str1[256];
	char style[8];
	double* xx;
	double* yy;
	double* zz;
	double* time = 0;
	int size, linewidth, symsize;

	switch (x.cmd) {
	case 0: 
        //xm_printf("got exit command!\n");
		stopQueueThread();
		//QApplication::quit();
		QApplication::exit();
		break;
	case 1: // figure
		n = b.getInt();
		pf->figure(n, 1);
		break;
	case 2: //  title
		b.getString(str1);
		pf->title(str1);
		break;
	case 3: //  xlabel
		b.getString(str1);
		pf->xlabel(str1);
		break;
	case 4: //  ylabel
		b.getString(str1);
		pf->ylabel(str1);
		break;
	case 5: // plot
		xx = b.getDPtr(); yy = b.getDPtr();
		size = b.getInt(); b.getString(str1); b.getString(style);

		linewidth = b.getInt(); symsize = b.getInt();
		time = b.getDPtr();
		pf->plot(xx, yy, size, str1, style, linewidth, symsize, time);
		sendReply(); //    notify 'host' that it can return from function and free the memory
		break;
	case 20: // plot  3D
		xx = b.getDPtr(); yy = b.getDPtr(); zz = b.getDPtr();
		size = b.getInt(); b.getString(str1); b.getString(style);

		linewidth = b.getInt(); symsize = b.getInt();
		time = b.getDPtr();
		pf->plot(xx, yy, zz, size, str1, style, linewidth, symsize, time);
		sendReply(); //    notify 'host' that it can return from function and free the memory
		//   since we supposed to copy everything already
		break;
	
	case 6:
	   mode = b.getInt();
	   pf->setmode(mode);
	   break;

#ifdef USEMARBLE
	case 7: // top view figure
		n = b.getInt();
		pf->figure(n, 2);
		break;
#endif
#ifdef USE_QT3D
	case 8: //  3D
		n = b.getInt();
		pf->figure(n, 3);
		break;
#endif

#ifdef ENABLE_UDP_SYNC
	case 9:
		xx = b.getDPtr(); yy = b.getDPtr(); zz = b.getDPtr();  time = b.getDPtr();  size = b.getInt();
		
		pf->enableCoordBroadcast(xx, yy, zz, time, size);
		sendReply(); //    notify 'host' that it can return from function and free the memory
		break;

	case 90: 
		pf->disableCoordBroacast();
		sendReply();
		break;
#endif


	case 10:  //   important status
		pf->setImportant(b.getInt() != 0);
		break;

	case 11:  //  close all plots
		pf->clear();
		break;
	case 12:  //  show main window
		pf->showMainWindow();
		break;

	default: xm_printf("QWTest::onInfo(): uncnown  command #%d\n", x.cmd);  break;
	};
}


void startQT_2();
void startQT_1() {
	//bt = new boost::thread(boost::bind(startQT_2, (void*)0));
	bt = new boost::thread(startQT_2);
	boost::this_thread::sleep(boost::posix_time::milliseconds(34));
}


void startQT_2() {
	mxassert(qApp123 == 0, "qtsMain::runQT error #1 ");
	//  create arguments for QApplication:
	static char argv0[MAX_PATH];
	getExeFilePath(argv0, MAX_PATH);

	//#ifdef _DEBUG
	if (argv0[0] == 0) {
		xm_printf("QTSMain::runQT() error #74635 \n");
		return;
	}
	else {
		xm_printf("QTSMain::runQT() start [%s]\n", argv0);
	}
	//#endif

	int argc = 1;
	static char* argv[2];
	argv[0] = argv0;

	// http://habrahabr.ru/post/188816/ :
	QStringList paths = QCoreApplication::libraryPaths();
	paths.append(".");
	paths.append("imageformats");
	paths.append("platforms");
	paths.append("sqldrivers");

	//paths.append("C:\\ProgramData\\qwtw");
#ifdef WIN32
	std::string qwtwSysPath = getCommonAppDataPath().append("\\qwtw");
#else
	std::string qwtwSysPath = getCommonAppDataPath().append("/qwtw");
#endif
	paths.append(qwtwSysPath.c_str());
	QCoreApplication::setLibraryPaths(paths);

	if (!qApp) {
		qApp123 = new QApplication(argc, argv);
	} else {
		qApp123 = qApp;
	}
	qApp123->setQuitOnLastWindowClosed(false);
	//Q_INIT_RESOURCE(plib1);
	//qtsMainWidget->setHidden(true);
	
	//#ifdef _DEBUG
	//qts2.start();
	if (qwTest == 0) {
		qwTest = new QWTest(bq);
	}
	qwTest->start();
	
    //xm_printf("\tQApplication created; starting QT \n");
	//#endif
	
	qApp123->exec();

    //xm_printf("\tQApplication exec finished \n");
}
*/

/*
void startQWT() {
	int ok;
	bq = new LRBuf<BQInfo, 16>();
	xm_printf("qwtw: 'startQWT' starting \n");
	if (qwtController == 0) {
		qwtController = new QWController();
	}
	qwtController->init(bq);

	qRegisterMetaType<BQInfo>();
	startQT_1();
}

void stopQWT() {
	if (qwtController != 0) {
		qwtController->close();
	}
}
*/



//  new interface below ==============================================================================



static std::thread* qt2Thread = 0;
//static QPointer<QCoreApplication> qt2App = nullptr;
static QPointer<QApplication> qt2App = nullptr;
static std::condition_variable q2_loading_cv;
static std::mutex q2_loading_mutex;
std::atomic<int> q2_started(0);

Worker::Worker(): pf(nullptr) {
	/*hello();*/ 
}

void Worker::onQtAppClosing() {
	if (pf != nullptr) {
		//printf("Worker::onQtAppClosing()! \n \tdeleting 'pf'... \n");
		//delete pf;  


		//pf->onExit();

		//printf("OK!\n");
		//pf = nullptr;
		//std::cout << "Worker::onQtAppClosing() ends " << std::endl;
	}
}

Worker::~Worker() {
	if (pf != nullptr) {
		//xm_printf("Worker::~Worker() \n");
		//delete pf;  // this is not 100% OK here, not a QT thread
		//pf = nullptr;
		//std::cout << "Worker::~Worker(): pf was deleted!  this is not 100% OK here, not a QT thread" << std::endl;
	}
}

void Worker::hello() {
	// This is thread-safe, the method is invoked from the event loop
	if (!QMetaObject::invokeMethod(this, "helloImpl", Qt::QueuedConnection)) {
		std::cout << " cannot invoke helloImpl" << std::endl;
	}
}
void Worker::qwtshowmw() {
	if (!QMetaObject::invokeMethod(this, "qwtshowmwImpl", Qt::QueuedConnection)) {
		std::cout << " cannot invoke qwtshowmwImpl" << std::endl;
	}
}
#ifdef USEMARBLE
void Worker::topview(int n) {
	int rv;
	QMetaObject::invokeMethod(this, "topviewImpl", Qt::BlockingQueuedConnection, Q_RETURN_ARG(int, rv), Q_ARG(int, n));
}
#endif
int Worker::qVersion(char* vstr, int vstr_size) {
#ifdef WIN32
	return xqversion(vstr, vstr_size - 2, qwtwLibModule);
#else
	strcpy(vstr, " linux? ");
	return 7;
#endif
}
int Worker::qwttitle(const char* s) {  //  blocking because have to copy param "s"
	int rv;
	//QString s1 = s;
	if (!QMetaObject::invokeMethod(this, "qwttitleImpl", Qt::BlockingQueuedConnection, Q_RETURN_ARG(int, rv), Q_ARG(const char*, s))) {
		std::cout << " cannot invoke qwttitleImpl" << std::endl;
	}
	return rv;
}
void Worker::qwtEnableCoordBroadcast(double* x, double* y, double* z, double* time, int size) {
	if (!QMetaObject::invokeMethod(this, "qwtEnableCoordBroadcastImpl", Qt::BlockingQueuedConnection, 
		Q_ARG(double*, x), Q_ARG(double*, y), Q_ARG(double*, z),
		Q_ARG(double*, time), Q_ARG(int, size))) {
		std::cout << " cannot invoke qwtEnableCoordBroadcastImpl" << std::endl;
	}

}
void Worker::qwtDisableCoordBroadcast() {
	if (!QMetaObject::invokeMethod(this, "qwtDisableCoordBroadcastImpl", Qt::QueuedConnection)) {
		std::cout << " cannot invoke qwtDisableCoordBroadcastImpl" << std::endl;
	}
}
void Worker::qwtsetimpstatus(int status) {
	//int rv;
	if (!QMetaObject::invokeMethod(this, "qwtsetimpstatusImpl", Qt::QueuedConnection, Q_ARG(int, status))) {
		std::cout << " cannot invoke qwtsetimpstatusImpl" << std::endl;
	}
}

void Worker::qwtxlabel(const char* s) {
	int rv;
	if (!QMetaObject::invokeMethod(this, "qwtxlabelImpl", Qt::BlockingQueuedConnection, Q_ARG(const char*, s))) {
		std::cout << " cannot invoke qwtxlabelImpl" << std::endl;
	}
}

void Worker::qwtylabel(const char* s) {
	int rv;
	if (!QMetaObject::invokeMethod(this, "qwtylabelImpl", Qt::BlockingQueuedConnection,  Q_ARG(const char*, s))) {
		std::cout << " cannot invoke qwtylabelImpl" << std::endl;
	}
}



void Worker::qwtplot2(double* x, double* y, int size, const char* name, const char* style, int lineWidth, int symSize, double* time) {
	int rv;
	if (!QMetaObject::invokeMethod(this, "qwtplot2Impl", Qt::BlockingQueuedConnection, Q_RETURN_ARG(int, rv),
		Q_ARG(double*, x), Q_ARG(double*, y), Q_ARG(int, size), Q_ARG(const char*, name), Q_ARG(const char*, style),
		Q_ARG(int, lineWidth), Q_ARG(int, symSize), Q_ARG(double*, time))) {

		std::cout << " cannot invoke qwtplot2Impl" << std::endl;
	}
}

void Worker::qwtclear() {
	if (!QMetaObject::invokeMethod(this, "qwtclearImpl", Qt::QueuedConnection)) {
		std::cout << " cannot invoke qwtclearImpl" << std::endl;
	}
}

void Worker::qwtfigure(int n) {
//	int rv;
	if (!QMetaObject::invokeMethod(this, "qwtfigureImpl", Qt::QueuedConnection,  Q_ARG(int, n))) {
		std::cout << " cannot invoke qwtfigureImpl" << std::endl;
	}
}

void Worker::qwtplot(double* x, double* y, int size, const char* name, const char* style, int lineWidth, int symSize) {
	int rv;
	if (!QMetaObject::invokeMethod(this, "qwtplotImpl", Qt::BlockingQueuedConnection, 
		Q_ARG(double*, x), Q_ARG(double*, y), Q_ARG(int, size), Q_ARG(const char*, name), Q_ARG(const char*, style),
		Q_ARG(int, lineWidth), Q_ARG(int, symSize))) {

		std::cout << " cannot invoke qwtplotImpl" << std::endl;
	}
}


Q_INVOKABLE int Worker::qwttitleImpl(const char* s) {
	pf->title(s);
	return 24;
}

Q_INVOKABLE int Worker::qwtplot2Impl(double* x, double* y, int size, const char* name, 
		const char* style, int lineWidth, int symSize, double* time) {
	pf->setmode(3);
	pf->plot(x, y, size, name, style, lineWidth, symSize, time);
	return 0;
}
Q_INVOKABLE void Worker::qwtEnableCoordBroadcastImpl(double* x, double* y, double* z, double* time, int size) {
	pf->enableCoordBroadcast(x, y, z, time, size);
}
Q_INVOKABLE void Worker::qwtDisableCoordBroadcastImpl() {
	pf->disableCoordBroacast();
}

Q_INVOKABLE void Worker::qwtclearImpl() {
	pf->clear();
}

Q_INVOKABLE void Worker::qwtsetimpstatusImpl(int status) {
	pf->setImportant(status != 0);
}

Q_INVOKABLE void Worker::qwtxlabelImpl(const char* s) {
	pf->xlabel(s);
}
Q_INVOKABLE void Worker::qwtylabelImpl(const char* s) {
	pf->ylabel(s);
}

Q_INVOKABLE int Worker::qwtfigureImpl(int n) {
	JustAplot*  test = pf->figure(n, 1);
	return (test == 0) ? 1 : 0;
}
Q_INVOKABLE void Worker::qwtplotImpl(double* x, double* y, int size, const char* name, const char* style, int lineWidth, int symSize) {
	pf->plot(x, y, size, name, style, lineWidth, symSize);
}



Q_INVOKABLE int Worker::qtstartImpl() {
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
Q_INVOKABLE void Worker::qwtshowmwImpl() {
	if (pf != nullptr) {
		pf->showMainWindow();
	}
}
#ifdef USEMARBLE
Q_INVOKABLE int Worker::topviewImpl(int n) {
	JustAplot* test = pf->figure(n, 2);
	return (test == 0) ? 1 : 0;
}
#endif

int Worker::qtstart(bool wait) {
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

//static Worker worker_;
static QPointer<Worker> q2worker = nullptr;
 
static int argc = 1;
static char* argv[8];
static char argv0[MAX_PATH];

void startQt2Thread() {
	//  create arguments for QApplication:
	printf("startQt2Thread() started! \n");

	using namespace std::chrono_literals;
	std::this_thread::sleep_for(100ms);
	printf("now starting QT5 .. \n");

	getExeFilePath(argv0, MAX_PATH);
	if (argv0[0] == 0) {
		xm_printf("startQt2Thread   error #74635 \n");
		argc = 0;
		argv[0] = 0;
		argv[1] = 0;
	}	else {
		//xm_printf("startQt2Thread: start [%s]\n", argv0);
		argv[0] = argv0;
		argv[1] = 0;
		argv[2] = 0;
	}
#ifdef WIN32
	if (qwtwLibModule == 0) {
		xm_printf("qwtwLibModule == 0\n");
	} 	else {
		char dllPath[MAX_PATH];
		DWORD dw = GetModuleFileNameA((HMODULE)(qwtwLibModule), dllPath, MAX_PATH);
		dllPath[MAX_PATH - 1] = 0; dllPath[MAX_PATH - 2] = 0;
		xm_printf("qwtw started by %s from %s\n", argv0, dllPath);

		// lets adjust PATH a little bit at thsi point:
		const char* env_p = std::getenv("PATH");
		using namespace boost::filesystem;
		std::string e = (path(dllPath)).parent_path().string();
		const char* vcpkg_path = e.c_str();
		if (strstr(env_p, vcpkg_path)) {
			xm_printf("PATH looks like already OK\n");
		} else {
			std::string env_p1 = e + ";" + env_p;
#ifdef WIN32
			//std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
			//std::wstring wide = converter.from_bytes(env_p1);
			//BOOL test1 = SetEnvironmentVariable(L"PATH", wide.c_str());
			//if (!test1) {
			//	DWORD test2 = GetLastError();
			//	xm_printf("can not change PATH (error %d) \n\n", test2);
			//}

			_putenv_s("PATH", env_p1.c_str());
#else
			setenv("PATH", env_p1.c_str(), 1);
#endif
			//xm_printf("%s added to the PATH\n", vcpkg_path);

			//xm_printf("\nPATH =  %s\n\n", std::getenv("PATH"));

			//wchar_t wtmp[4096];
			//GetEnvironmentVariable(L"PATH", wtmp, 4096);
			//std::string tmp = converter.to_bytes(std::wstring(wtmp));
			//xm_printf("\none more PATH = %s\n\n", tmp.c_str());
		}
		

	}
#endif
/*
	// http://habrahabr.ru/post/188816/ :
	QStringList paths = QCoreApplication::libraryPaths();
	paths.append(".");
	paths.append("imageformats");
	paths.append("platforms");
	paths.append("sqldrivers");

#ifdef WIN32
	std::string qwtwSysPath = getCommonAppDataPath().append("\\qwtw");
#else
	std::string qwtwSysPath = getCommonAppDataPath().append("/qwtw");
#endif
	paths.append(qwtwSysPath.c_str());
	QCoreApplication::setLibraryPaths(paths);
*/
	q2_loading_mutex.lock();
	if (!qApp) {
		//xm_printf("PATH before new QApplication: %s\n\n", std::getenv("PATH"));
		qt2App = new QApplication(argc, argv);
		printf("QApplication created!! \n");

	}	else {
		qt2App = qApp;
	}
	qt2App->setQuitOnLastWindowClosed(false);
	Worker worker_;
	q2worker = &worker_;

	q2worker->qtstart(false);
	QObject::connect(qt2App, SIGNAL(aboutToQuit()), q2worker, SLOT(onQtAppClosing()));

	//std::cout << "runQt2Thread() started " << std::endl;
	q2_started = 1;
	q2_loading_mutex.unlock();
	q2_loading_cv.notify_all();
	//printf("running QT exec ..... \n");
	
	
	q2worker = nullptr;
	q2_loading_mutex.lock(); 
	//std::cout << "startQt2Thread() STOPPED " << std::endl;
	q2_started = 2;
	q2_loading_mutex.unlock();
	q2_loading_cv.notify_all();
}

void stopQt2Thread() {
	std::cout << "stopQt2Thread(): q2_started = " << q2_started << std::endl;
	using namespace std::chrono_literals;
	if ((q2_started == 1)) { //  looks like QT thread is still running
		if (qt2App != nullptr) { // try to make QT to stop
			std::cout << "stopQt2Thread(): stopping the thread..invokeMethod(qt2App->quit)" << std::endl;
			QMetaObject::invokeMethod(qt2App, "quit", Qt::BlockingQueuedConnection); // QueuedConnection
			//std::this_thread::sleep_for(100ms);
			xm_printf("now calling 'qt2App->quit()'\n");
			qt2App->quit();
			xm_printf("now calling 'qt2App->exit(0)'\n");
			qt2App->exit(0);
			//std::this_thread::sleep_for(100ms);
		}
		// wait some more time:
		//std::cout << "stopQt2Thread(): stopping the thread...2 " << std::endl;
		std::unique_lock<std::mutex> lk(q2_loading_mutex);
		if (q2_started != 1) {
			//std::cout << "stopQt2Thread(): QT stopped already; q2_started = " << q2_started << std::endl;
			//qt2Thread.join(); 
			//std::cout << "stopQt2Thread(): bye. " << std::endl;
		} else {
			q2_loading_cv.wait_for(lk, 150ms, [] {return (q2_started != 1); });
			std::cout << "stopQt2Thread(): stopping the thread...3; q2_started =  " << q2_started << std::endl;
			//qt2Thread.join();
			std::cout << "stopQt2Thread(): done stopping the thread; q2_started =  " << q2_started << std::endl;
		}
	}
	q2_started = 0; 
	//printf("stopQt2Thread() finished! \n\n");
}

extern "C" {
	qwtwc_API void kyleHello() {
		q2worker->hello();
	}
	qwtwc_API	int qwtversion(char* vstr, int vstr_size) {
		//std::unique_lock<std::mutex> lk(q2_loading_mutex);
		//q2_loading_cv.wait(lk, [] {return (q2_started != 0); });
		qtstart();
		return q2worker->qVersion(vstr, vstr_size);
	}

	qwtwc_API	int qtstart() {
		if ((q2_started == 1)) return 1;

		// lets wait for the thread to start:
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(100ms); 
		std::unique_lock<std::mutex> lk(q2_loading_mutex);
		q2_loading_cv.wait(lk, [] {return (q2_started != 0); });

		//  at this point we supposed to have valid  q2worker
		if (q2worker == nullptr) {
			std::cout << " error in qtstart(): (q2worker == nullptr)" << std::endl;
			return 5;
		}
		//  now lets wait for the internal QT message loop to start:
		int u = q2worker->qtstart(true);

		//if (const char* env_p = std::getenv("PATH")) {
		//	xm_printf("qwtw started already; PATH = %s\n\n", env_p);
		//}
		//std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		//wchar_t wtmp[4096];
		//GetEnvironmentVariable(L"PATH", wtmp, 4096);
		//std::string tmp = converter.to_bytes(std::wstring(wtmp));
		//xm_printf("\n  one more PATH #3: %s\n\n", tmp.c_str());

		return u;
	}

	qwtwc_API void qwtfigure(int n) {
		q2worker->qwtfigure(n);
	}

	qwtwc_API void qwtplot(double* x, double* y, int size, const char* name, const char* style, int lineWidth, int symSize) {
		q2worker->qwtplot(x, y, size, name, style, lineWidth, symSize);
	}

	qwtwc_API void qwtplot2(double* x, double* y, int size, const char* name, const char* style, int lineWidth, int symSize, double* time) {
		//	qwtController->setmode(3);
		//	qwtController->plot(x, y, size, name, style, lineWidth, symSize, time);
		q2worker->qwtplot2(x, y, size, name, style, lineWidth, symSize, time);
	}

	qwtwc_API void qwtxlabel(const char* s) {
		q2worker->qwtxlabel(s);
	}
	
	qwtwc_API void qwtylabel(const char* s) {
		q2worker->qwtylabel(s);
	}


	qwtwc_API 	void qwtshowmw() {
		qtstart();
		return q2worker->qwtshowmw();
	}
	//qwtwc_API void qwtclose() {
	//	stopQt2Thread();
	//}

	qwtwc_API 	void qwtclear() {
		q2worker->qwtclear();
	}

#ifdef USEMARBLE
	qwtwc_API void topview(int n) {
		q2worker->topview(n);
	}
#endif
	qwtwc_API void qwttitle(const char* s) {
		int rv = q2worker->qwttitle(s);
	}

#ifdef ENABLE_UDP_SYNC
	qwtwc_API 	void qwtEnableCoordBroadcast(double* x, double* y, double* z, double* time, int size) {
		q2worker->qwtEnableCoordBroadcast(x, y, z, time, size);
	}

	qwtwc_API 	void qwtDisableCoordBroadcast() {
		q2worker->qwtDisableCoordBroadcast();
	}
	qwtwc_API 	void qwtsetimpstatus(int status) {
		q2worker->qwtsetimpstatus(status);
	}
#endif

}

#ifdef WIN32
BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
) {
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		qwtwLibModule = hModule;
		++prCounter;
		//printf("DLL main: DLL_PROCESS_ATTACH; prCounter = %d\n", prCounter);
		break;
	case DLL_THREAD_ATTACH:
		++trCounter;
		//printf("DLL main: DLL_THREAD_ATTACH; trCounter = %d\n", trCounter);
		break;
	case DLL_THREAD_DETACH:
		--trCounter;
		//printf("DLL main: DLL_THREAD_DETACH; trCounter = %d\n", trCounter);
		break;
	case DLL_PROCESS_DETACH:
		--prCounter;
		if (qt2App != nullptr) {
			//QMetaObject::invokeMethod(qt2App, "quit", Qt::QueuedConnection);
			//qt2App->quit();
			//qt2App->exit(0);
		}
		//printf("DLL main: DLL_PROCESS_DETACH; prCounter = %d\n", prCounter);
		break;
		break;
	}
	return TRUE;
}

#endif



class DLLStarter {
public:
    DLLStarter() {
		xm_printf("starting another thread for QT \n");
		std::thread ttmp(startQt2Thread);
		qt2Thread = new std::thread();
		ttmp.swap(*qt2Thread);
		qt2Thread->detach();
    } 

    ~DLLStarter() {
		std::cout << "~DLLStarter()! started" << std::endl;
		stopQt2Thread();
		xm_printf("~DLLStarter()!  finished!!\n");
		//delete thread;
		//thread = 0;
		
#ifdef WIN32
		//stopQWT();
#endif
    }

};
static DLLStarter ourDllStarter;


