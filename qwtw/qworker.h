

#pragma once

#include <QObject>
#include <mutex>
#include <condition_variable>
//#include <QPointer>
//#include <QThread>

class XQPlots;

// https://stackoverflow.com/questions/25025168/event-loop-in-qt-based-dll-in-a-non-qt-application
class QWorker : public QObject {
	Q_OBJECT
   
	Q_INVOKABLE void helloImpl() { printf("I'm alive.\n"); }
	Q_INVOKABLE int qtstartImpl();
	Q_INVOKABLE void qwtshowmwImpl();
#ifdef USEMARBLE
	Q_INVOKABLE int topviewImpl(int n);
#endif
	Q_INVOKABLE int qwttitleImpl(const char* s);
	Q_INVOKABLE int qwtplot2Impl(double* x, double* y, int size, const char* name, const char* style, int lineWidth, int symSize, double* time);
	Q_INVOKABLE void qwtEnableCoordBroadcastImpl(double* x, double* y, double* z, double* time, int size);
	Q_INVOKABLE void qwtDisableCoordBroadcastImpl();
	Q_INVOKABLE void qwtsetimpstatusImpl(int impStatus);
	Q_INVOKABLE void qwtxlabelImpl(const char* s);
	Q_INVOKABLE void qwtylabelImpl(const char* s);
	Q_INVOKABLE int qwtfigureImpl(int n);
	Q_INVOKABLE void qwtplotImpl(double* x, double* y, int size, const char* name, const char* style, int lineWidth, int symSize);
	Q_INVOKABLE void qwtclearImpl();

public:
	//std::mutex appMutex;///< for waiting on QT app 
	//std::condition_variable appV; ///< for waiting on QT app 

	void hello();
	QWorker(); //{  }
	~QWorker();
	int qVersion(char* vstr, int vstr_size);
	int qtstart(bool wait);
	void qwtshowmw();
	int qwttitle(const char* s);
	void qwtplot2(double* x, double* y, int size, const char* name, const char* style, int lineWidth, int symSize, double* time);
	void qwtEnableCoordBroadcast(double* x, double* y, double* z, double* time, int size);
	void qwtDisableCoordBroadcast();
	void qwtsetimpstatus(int impStatus);
	void qwtxlabel(const char* s);
	void qwtylabel(const char* s);
	void qwtclear();
	void qwtfigure(int n);
	void qwtplot(double* x, double* y, int size, const char* name, const char* style, int lineWidth, int symSize);
	//void appMutexLock

#ifdef USEMARBLE
	void topview(int n);
#endif
public slots:
	void onQtAppClosing();

private:
	
	XQPlots* pf;
    
};

