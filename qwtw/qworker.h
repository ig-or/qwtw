

#pragma once

#include <QObject>
#include <mutex>
#include <condition_variable>
#include "qwtypes.h"
//#include <QPointer>
//#include <QThread>

class XQPlots;
struct SpectrogramInfo;

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
	Q_INVOKABLE int qwtSavePngImpl(int id, char* filename);
	Q_INVOKABLE int qwtfigureImpl(int n, unsigned int flags);
	Q_INVOKABLE int qwtSpectrogramImpl(int n, unsigned int flags);
	Q_INVOKABLE int qwtserviceImpl(int x);
	Q_INVOKABLE void qwtSetClipGroupImpl(int gr);
	Q_INVOKABLE int qwtplotImpl(double* x, double* y, int size, const char* name, const char* style, int lineWidth, int symSize);
	Q_INVOKABLE void qwtclearImpl();
	Q_INVOKABLE void qwtRemoveLineImpl(int key);
	Q_INVOKABLE int qwtChangeLineImpl(int id, double* x, double* y, double* z, double* time, int size);

	Q_INVOKABLE void qwtSetUdpCallbackImpl(OnUdpCallback cb);
	Q_INVOKABLE void qwtSetPickerCallbackImpl(OnPickerCallback cb);

#ifdef USEMATHGL
	Q_INVOKABLE int mglPlotImpl(int n);
	Q_INVOKABLE int mgl_lineImpl(int size, double* x, double* y, double* z, const char* name, const char* style);
	Q_INVOKABLE int mgl_meshImpl(int xSize, int ySize, 
		double xMin, double xMax, double yMin, double yMax, 
		double* data, const char* name,
		const char* style,
		int type);
	Q_INVOKABLE int spectrogram_info_impl(const SpectrogramInfo& info);
#endif

public:
	//std::mutex appMutex;///< for waiting on QT app 
	//std::condition_variable appV; ///< for waiting on QT app 

	void hello();
	#ifdef USEMARBLE
	QWorker(const std::string& mdp, const std::string& mpp);
	#else
	QWorker(); //{  }
	#endif
	~QWorker();
	int qVersion(char* vstr, int vstr_size);
	int qtstart(bool wait);
	void qwtshowmw();
	int qwttitle(const char* s);
	int qwtplot2(double* x, double* y, int size, const char* name, const char* style, int lineWidth, int symSize, double* time);
	void qwtEnableCoordBroadcast(double* x, double* y, double* z, double* time, int size);
	void qwtDisableCoordBroadcast();
	void qwtsetimpstatus(int impStatus);
	void qwtxlabel(const char* s);
	void qwtylabel(const char* s);
	void qwtclear();
	void qwtSetUdpCallback(OnUdpCallback cb);
	void qwtSetPickerCallback(OnPickerCallback cb);
	int qwtfigure(int n, unsigned int flags);
	int qwtSpectrogram(int n, unsigned int flags);
	int qwtservice(int x);
	void qwtSetClipGroup(int gr);
	void qwtRemoveLine(int key);
	int qwtSavePng(int id, char* filename);
	int qwtChangeLine(int id, double* x, double* y, double* z, double* time, int size);
	int qwtplot(double* x, double* y, int size, const char* name, const char* style, int lineWidth, int symSize);
	//void appMutexLock

#ifdef USEMARBLE
	int mapview(int n);
#endif
#ifdef USEMATHGL
	int mglPlot(int n);
	void mgl_line(int size, double* x, double* y, double* z, const char* name, const char* style);
	void mgl_mesh(int xSize, int ySize, 
		double xMin, double xMax, double yMin, double yMax, 
		double* data, const char* name,
		const char* style,
		int type);
#endif
	void spectrogram_info(const SpectrogramInfo& info);
public slots:
	void onQtAppClosing();

private:
	
	XQPlots* pf = 0;
	#ifdef USEMARBLE
	std::string mdPath; ///< marble data path
	std::string mpPath; ///< marble plugin path
	#endif
    
};

