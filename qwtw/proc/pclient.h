
#pragma once

#include "qwproc.h"
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

struct SHMTest {

	SHMTest();

	/**  init and start everything.
	 * \param mdp marble data path
	 * 	\return 0 if all is OK. 
	**/
#ifdef USEMARBLE
	int testInit(const std::string& mdp, int level = 1);
#else
	int testInit(int level = 1);
#endif
	void qwtfigure(int n);

	void stopQt();

	void qwttitle(const char* s);
	void qwtxlabel(const char* s);
	void qwtylabel(const char* s);
	void qwtclear();
	#ifdef USEMARBLE
	void qwtmap(int n);
	#endif
	void qsetloglevel(int level);
	void qwtsetimpstatus(int impStatus);
	void qwtplot(double* x, double* y, int size, const char* name, const char* style, 
	int lineWidth, int symSize);
	void qwtplot2(double* x, double* y, int size, const char* name, const char* style, 
	int lineWidth, int symSize, double* time);
#ifdef USE_QT3D
	void qwtplot3d(double* x, double* y, double* z, int size, const char* name, const char* style,
	int lineWidth, int symSize, double* time);
#endif
#ifdef ENABLE_UDP_SYNC
	void qwtEnableCoordBroadcast(double* x, double* y, double* z, double* time, int size);
	void qwtDisableCoordBroadcast();
#endif

	void qwtshowmw(); 

private:    

	void sendCommand(CmdHeader::QWCmd cmd, const char* text = 0);
	void sendCommand(CmdHeader::QWCmd cmd, int v);
	void resize(long long size);

	/**\param params proc parameters
	 * \return 0 if all is OK
	 * 
	**/
#ifdef USEMARBLE
	int startProc(const std::string& mdp);
#else
	int startProc();
#endif

	boost::interprocess::shared_memory_object shmCommand;
	boost::interprocess::shared_memory_object shmX;
	boost::interprocess::shared_memory_object shmY;
	boost::interprocess::shared_memory_object shmZ;
	boost::interprocess::shared_memory_object shmT;

	boost::interprocess::mapped_region commandReg;
	boost::interprocess::mapped_region xReg;
	boost::interprocess::mapped_region yReg;
	boost::interprocess::mapped_region zReg;
	boost::interprocess::mapped_region tReg;

	ProcData pd;
	int status;
};








