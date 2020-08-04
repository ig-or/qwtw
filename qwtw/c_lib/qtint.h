

#pragma once

#include <QObject>
#include <QPointer>
#include <QThread>
#include <QCoreApplication>

//#include "queue_block.h"
//#include "lrbuf.h"
//#include "qtswidget.h"
#include <iostream>

class XQPlots;

// https://stackoverflow.com/questions/25025168/event-loop-in-qt-based-dll-in-a-non-qt-application
class Worker : public QObject {
	Q_OBJECT
	Q_INVOKABLE void helloImpl() { std::cout << "I'm alive." << std::endl; }
	Q_INVOKABLE int qtstartImpl();
	Q_INVOKABLE void qwtshowmwImpl();
	Q_INVOKABLE int topviewImpl(int n);
	Q_INVOKABLE int qwttitleImpl(const char* s);
	Q_INVOKABLE int qwtplot2Impl(double* x, double* y, int size, const char* name, const char* style, int lineWidth, int symSize, double* time);
	Q_INVOKABLE void qwtEnableCoordBroadcastImpl(double* x, double* y, double* z, double* time, int size);
	Q_INVOKABLE void qwtDisableCoordBroadcastImpl();
	Q_INVOKABLE void qwtsetimpstatusImpl(int status);
	Q_INVOKABLE void qwtxlabelImpl(const char* s);
	Q_INVOKABLE void qwtylabelImpl(const char* s);
	Q_INVOKABLE int qwtfigureImpl(int n);
	Q_INVOKABLE void qwtplotImpl(double* x, double* y, int size, const char* name, const char* style, int lineWidth, int symSize);
	Q_INVOKABLE void qwtclearImpl();


public:
	void hello();
	Worker(); //{ /*hello();*/ }
	~Worker();
	int qVersion(char* vstr, int vstr_size);
	int qtstart(bool wait);
	void qwtshowmw();
	int qwttitle(const char* s);
	void qwtplot2(double* x, double* y, int size, const char* name, const char* style, int lineWidth, int symSize, double* time);
	void qwtEnableCoordBroadcast(double* x, double* y, double* z, double* time, int size);
	void qwtDisableCoordBroadcast();
	void qwtsetimpstatus(int status);
	void qwtxlabel(const char* s);
	void qwtylabel(const char* s);
	void qwtclear();
	void qwtfigure(int n);
	void qwtplot(double* x, double* y, int size, const char* name, const char* style, int lineWidth, int symSize);

#ifdef USEMARBLE
	void topview(int n);
#endif
public slots:
	void onQtAppClosing();

private:
	XQPlots* pf;
};
/*
class  WorkerHelper : public QObject {
	Q_OBJECT
public slots:
	void onQtAppClosing();
};
*/
/*
class AppThread : public QThread {
	// No need for the Q_OBJECT
	QPointer<QCoreApplication> m_app;
	//QPointer<QApplication> m_app;
	void run() Q_DECL_OVERRIDE;
	//using QThread::wait(); // This wouldn't work here.
public:
	AppThread() {}
	~AppThread();
};

*/

//class QTSMainWidget;
/*
class QWController {
public:
	QWController();
	/// @return 0 if all is OK
	int init(LRBuf<BQInfo, 16>* bq_);
	void figure(int n);  // 1
	void title(char* s); // 2
	void xlabel(char* s); // 3
	void ylabel(char* s); // 4
	void setmode(int mode); // 6
	void clear(); // 11
	void showMW();
	void setImportantStatus(int status); // 10
#ifdef USEMARBLE
	void figure_topview(int n);  // 7    (top view)
#endif
#ifdef USE_QT3D
	void figure_3d(int n); // 8     3D plot
#endif
#ifdef ENABLE_UDP_SYNC
	void qwtEnableCoordBroadcast(double* x, double* y, double* z, double* time, int size); // 9
	void qwtDisableCoordBroadcast(); // 90
#endif

	void plot(double* x, double* y, int size, char* name, const char* style, 
		  int lineWidth, int symSize, double* time = 0); // 5
	void plot(double* x, double* y, double* z,  int size, char* name, const char* style,
		int lineWidth, int symSize, double* time = 0);  // 20
	void close(); // NOT WORKING
	
private:
	bool ok;
	//QueueBlock<BQInfo, 16>* bq;
	LRBuf<BQInfo, 16>* bq;
	void send(unsigned char cmd, const char* b, int size, bool waitForReply = false);
};

extern QWController* qwtController;
*/
/*
class QWTest : public QTSMainWidget {
	Q_OBJECT
public:
	QWTest(LRBuf<BQInfo, 16>* bq_);
protected:
	void onInfo(BQInfo x);
private:
	XQPlots* pf;
};

*/







