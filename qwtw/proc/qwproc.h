

#pragma once

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>

#pragma pack(8)
struct CmdHeader {
	enum QWCmd {
		nothing = 0,
		exit = 1,
		changeSize,
		changeDataSize,
		qMW,
		qFigure,
		qRemoveLine,
		qMap, 
		qPlot,
		qPlot2, 
		qChangeLine,
		qTitle, 
		qXlabel, // 10
		qYlabel,
		qClear,
		qImpStatus,
		qEnableBC,
		qDisableBC,
		qSetLogLevel,
		qMglPlot, // 17
		qMglLine, // 18
		qMglMesh, // 19

		qwcmdSize
	};
	enum {
		nameSize = 128
	};
	int cmd; ///< QWCmd
	int segSize; ///< size of each memory segment in 8th (how many numbers maximum..)
	int dataSize;

	long long size;
	int lineWidth;
	long long symSize;
	int test;
	char style[32];
	char name[nameSize];

	int xSize, ySize, type;
	double xMin, xMax, yMin, yMax;

	int timeIsGood, zIsGood;

	boost::interprocess::interprocess_mutex      mutex;
	boost::interprocess::interprocess_condition  cmdWait;
	boost::interprocess::interprocess_condition  workDone;
	CmdHeader() {}
};


struct ProcData {
	static constexpr const char* shmNames[] = {
		"QWTWCommand", "QWTW_x", "QWTW_y", "QWTW_z", "QWTW_t", "QWTW_data"
	};
	CmdHeader* hdr;
	double*	x;
	double* y;
	double* z;
	double* t;
	double* data;
};

#pragma pack()

/** get location for all the 'system' files
\param p[in] put location here
\param pSize size of the 'p'
\return false if error, true if all is OK
*/
bool getFolderLocation(char* p, int pSize);

/** 
\return 0 if proc is not running; 1 if running; smth else if error
*/
int checkProcRunning();
extern int xmPrintLevel;
int xmprintf(int level, const char * _Format, ...);

