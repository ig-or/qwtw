
#pragma once

#include "qwproc.h"
#include "qwtypes.h"
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

struct SHMTest {

	SHMTest();

	/**  init and start everything.
	 * \param mdp marble data path
	 * 	\return 0 if all is OK. 
	**/
#ifdef USEMARBLE
	int testInit(const std::string& mdp, const std::string& mpp, int level = 0);
#else
	int testInit(int level = 0);
#endif
	int qwtfigure(int n);
	void qwtClipGroup(int gr);

	void stopQt();

	void qwttitle(const char* s);
	void qwtxlabel(const char* s);
	void qwtylabel(const char* s);
	void qwtclear();
	#ifdef USEMARBLE
	int qwtmap(int n);
	#endif
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

private:    

	int sendCommand(CmdHeader::QWCmd cmd, const char* text = 0);
	int sendCommand(CmdHeader::QWCmd cmd, int v);
	void resize(long long size);
	void resizeData(long long size);

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








