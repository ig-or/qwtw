

#pragma once

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>

#pragma pack(8)
struct CmdHeader {
	enum QWCmd {
		nothing = 0,
		exit = 1,

		qwcmdSize
	};
	enum {
		nameSize = 128
	};
	int cmd; ///< QWCmd
	int segSize; ///< size of each memory segment in 8th (how many numbers maximum..)

	int size;
	int lineWidth;
	int symSize;
	int test;
	char style[4];
	char name[nameSize];

	boost::interprocess::interprocess_mutex      mutex;
	boost::interprocess::interprocess_condition  cmdWait;
	boost::interprocess::interprocess_condition  workDone;
};


struct ProcData {
	CmdHeader* hdr;
	double*	x;
	double* y;
	double* t;
};

#pragma pack()

