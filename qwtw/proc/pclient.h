
#pragma once

#include "qwproc.h"
#include "qwtypes.h"
#include "qwtw.h"
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <thread>
#include <mutex>
//#include <sha
#include <condition_variable>

struct SHMTest {

	SHMTest();
	~SHMTest();

	/**  init and start everything.
	 * \param mdp marble data path
	 * 	\return 0 if all is OK. 
	**/
#ifdef USEMARBLE
	int testInit(const std::string& mdp, const std::string& mpp, int level = 0);
#else
	int testInit(int level = 0);
#endif
	int qwtfigure(int n, unsigned int flags);
	void qwtClipGroup(int gr);

	void stopQt();

	void qwttitle(const char* s);
	void qwtxlabel(const char* s);
	void qwtylabel(const char* s);
	void qwtclear();
	#ifdef USEMARBLE
	int qwtmap(int n);
	#endif
	int qwtservice(int x);
	#ifdef USEMATHGL
	int qwtmgl(int n);
	void qwtmgl_line(int size, double* x, double* y, double* z, const char* name, const char* style);
	void qwtmgl_mesh(const MeshInfo& info);
	#endif
	void qsetloglevel(int level);
	void qwtsetimpstatus(int impStatus);
	int qwtplot(double* x, double* y, int size, const char* name, const char* style, int lineWidth, int symSize);
	int qwtplot2(double* x, double* y, int size, const char* name, const char* style, int lineWidth, int symSize, double* time);

	/// <summary>
	///   remove a line from a plot
	/// </summary>
	/// <param name="id"> id of thie line</param>
	void qwtremove(int id);
	int qwtchange(int id, double* x, double* y, double* z, double* time, int size);
#ifdef USE_QT3D
	void qwtplot3d(double* x, double* y, double* z, int size, const char* name, const char* style,
	int lineWidth, int symSize, double* time);
#endif
#ifdef ENABLE_UDP_SYNC
	void qwtEnableCoordBroadcast(double* x, double* y, double* z, double* time, int size);
	void qwtDisableCoordBroadcast();
#endif

	void qwtshowmw(); 
	void setCB(OnPCallback cb);
	void setCBTest1(CBTest_1 cb);
	void setCBTest2(CBTest_2 cb);

private:    

	int sendCommand(CmdHeader::QWCmd cmd, const char* text = 0);
	int sendCommand(CmdHeader::QWCmd cmd, int v, unsigned int flags = 0);
	void resize(long long size);
	void resizeData(long long size);

	bool needStopCallbackThread = false;
	std::thread cbThread;
	/// <summary>
	/// callback thread. callbacks will be called from here.
	/// </summary>
	void cbThreadF();
	OnPCallback pCallback = 0;
	CBTest_1    cbTest1 = 0;
	CBTest_2	cbTest2 = 0;

	std::thread cbThread_2;
	CBPickerInfo cpInfo;
	void cbThreadF_2();
	std::mutex cbiMutex_2;
	std::mutex cbiMutex_3;
	std::condition_variable cbiReady;

	std::unique_ptr<CmdSync> cmdSync = nullptr;

	/**\param params proc parameters
	 * \return 0 if all is OK
	 * 
	**/
#ifdef USEMARBLE
	int startProc(const std::string& mdp, const std::string& mpp, int level);
#else
	int startProc(int level);
#endif

	boost::interprocess::shared_memory_object shmCommand;
	boost::interprocess::shared_memory_object shmX;
	boost::interprocess::shared_memory_object shmY;
	boost::interprocess::shared_memory_object shmZ;
	boost::interprocess::shared_memory_object shmT;
	boost::interprocess::shared_memory_object shmData;

	boost::interprocess::mapped_region commandReg;
	boost::interprocess::mapped_region xReg;
	boost::interprocess::mapped_region yReg;
	boost::interprocess::mapped_region zReg;
	boost::interprocess::mapped_region tReg;
	boost::interprocess::mapped_region dataReg;

	ProcData pd;
	int status;
};








