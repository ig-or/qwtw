

#pragma once

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>

#pragma pack(8)
struct CmdHeader {
	enum QWCmd {
		nothing = 0,
		exit = 1,
		changeSize,
		qMW,
		qFigure,
		qPlot,
		qPlot2, 
		qTitle, 
		qXlabel,
		qYlabel,
		qClear,
		qImpStatus,
		qwcmdSize
	};
	enum {
		nameSize = 128
	};
	int cmd; ///< QWCmd
	int segSize; ///< size of each memory segment in 8th (how many numbers maximum..)

	long long size;
	int lineWidth;
	long long symSize;
	int test;
	char style[8];
	char name[nameSize];

	boost::interprocess::interprocess_mutex      mutex;
	boost::interprocess::interprocess_condition  cmdWait;
	boost::interprocess::interprocess_condition  workDone;
	CmdHeader() {}
};


struct ProcData {
	static constexpr const char* shmNames[] = {"QWTWCommand", "QWTW_x", "QWTW_y", "QWTW_t"};
	CmdHeader* hdr;
	double*	x;
	double* y;
	double* t;
};



#pragma pack()

