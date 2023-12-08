

#pragma once

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/named_condition.hpp>
#include "qwtypes.h"

#pragma pack(8)

struct CmdHeader {
	enum QWCmd {
		nothing = 0,
		exit = 1,
		changeSize,
		changeDataSize,
		qMW,
		qFigure,
		qSpectrogram,
		qSpectrogramInfo,
		qRemoveLine,
		qMap, 
		qPlot,
		qPlot2, 
		qChangeLine,   // 
		qTitle, 
		qXlabel, 
		qYlabel,
		qClear,
		qImpStatus,
		qEnableBC,
		qDisableBC,
		qSetLogLevel,     // 
		qMglPlot, // 
		qMglLine, // 
		qMglMesh, // 
		qSetClipGroup, // 
		qSetUdpCallback,  //  
		qSetPickerCallback,  // 
		qService,			// 

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
	unsigned int flags;
	char style[32];
	char name[nameSize];

	int xSize, ySize, type;
	double xMin, xMax, yMin, yMax;

	int timeIsGood, zIsGood;
	//void* fTest;

	boost::interprocess::interprocess_mutex      mutex;
	boost::interprocess::interprocess_condition  cmdWait;
	boost::interprocess::interprocess_condition  workDone;


	CBPickerInfo   cbInfo;
	//boost::interprocess::interprocess_mutex      cbInfoMutex;
	//boost::interprocess::interprocess_condition  cbWait;  ///   callback wait
	CmdHeader() {}
};

///    this is for callback function sync
struct CmdSync {
	boost::interprocess::named_mutex		cbInfoMutex;
	boost::interprocess::named_condition	cbWait;
	CmdSync();
	~CmdSync();
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

