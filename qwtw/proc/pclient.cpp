
#include "xstdef.h"
#include "pclient.h"
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/process/spawn.hpp>
#include <boost/process/search_path.hpp>
#include <thread>
#include <chrono>
#include <cstdlib>

SHMTest::SHMTest(): status(5) {

}

void SHMTest::qwtfigure(int n) {
	if (status != 0) return;
	sendCommand(CmdHeader::qFigure, n);
}
void SHMTest::qwtsetimpstatus(int status) {
	if (status != 0) return;
	sendCommand(CmdHeader::qImpStatus, status);
}

int SHMTest::startProc() {
	xmprintf(2, "starting proc.. \n");
	using namespace std::chrono_literals;
	using namespace boost::filesystem;
	namespace bp = boost::process;
	std::error_code ec;
#ifdef WIN32
	const char* procName = "qwproc.exe";
#else
	const char* procName = "qwproc";
#endif
	//int ret = std::system(procName);
	path pa;
	try {
		pa = bp::search_path(procName);
	} catch  (std::exception& ex) {
		xmprintf(0, "cannot find %s in PATH \n", procName);
		return 2;
	}
	try {
		bp::spawn(pa);
		std::this_thread::sleep_for(275ms);
	}	catch (std::exception& ex) {
		xmprintf(0, "cannot start process %s from (%s) (%s) \n", procName, pa.string().c_str(), ex.what());
		return 1;
	}
	xmprintf(2, "SHMTest::startProc() exiting \n");
	return 0;
}

int SHMTest::testInit() {
	xmprintf(2, "starting SHMTest::testInit()\n");
	if (status == 0) {
		//return 0;
		xmprintf(2, "starting SHMTest::testInit()  (status == 0)\n");
	}
	using namespace boost::interprocess;
	
	int test = checkProcRunning();
	if (test == 0) {  //  not running
		startProc();
		//  try one more time
		test = checkProcRunning();
		if (test == 0) {  // still not running
			status = 1;
			return 1; //  cannot start the program  (not installed?)
		}
	}

	try {
		shared_memory_object shmCommand_(open_only, ProcData::shmNames[0], read_write);
		shmCommand.swap(shmCommand_);
	} catch(interprocess_exception &ex) { // proc not started?  something is not OK
		xmprintf(0, "SHMTest::testInit():  proc not started? cannot connect to the SHM \n");
		status = 2;
		return 2;
	}

	shared_memory_object shmX_(open_only, ProcData::shmNames[1], read_write);
	shared_memory_object shmY_(open_only, ProcData::shmNames[2], read_write);
	shared_memory_object shmZ_(open_only, ProcData::shmNames[3], read_write);
	shared_memory_object shmT_(open_only, ProcData::shmNames[4], read_write);
	
	shmX.swap(shmX_);
	shmY.swap(shmY_);
	shmZ.swap(shmZ_);
	shmT.swap(shmT_);

	shmCommand.truncate(sizeof(CmdHeader));
	mapped_region commandReg_ = mapped_region(shmCommand, read_write);
	commandReg.swap(commandReg_);
	pd.hdr = static_cast<CmdHeader*>(commandReg.get_address());
	{
		scoped_lock<interprocess_mutex> lock(pd.hdr->mutex);
		long long segSize = pd.hdr->segSize;
		shmX.truncate(segSize * sizeof(double));
		shmY.truncate(segSize * sizeof(double));
		shmZ.truncate(segSize * sizeof(double));
		shmT.truncate(segSize * sizeof(double));
	}

	mapped_region xReg_ = mapped_region(shmX, read_write);
	mapped_region yReg_ = mapped_region(shmY, read_write);
	mapped_region zReg_ = mapped_region(shmZ, read_write);
	mapped_region tReg_ = mapped_region(shmT, read_write);

	xReg.swap(xReg_);
	yReg.swap(yReg_);
	zReg.swap(zReg_);
	tReg.swap(tReg_);

	pd.x = static_cast<double*>(xReg.get_address());
	pd.y = static_cast<double*>(yReg.get_address());
	pd.z = static_cast<double*>(zReg.get_address());
	pd.t = static_cast<double*>(tReg.get_address());
	status = 0;
	xmprintf(2, "starting SHMTest::testInit() finished\n");
	return 0;
}

void SHMTest::stopQt() {
	if (status != 0) return;
	using namespace boost::interprocess;
	
	scoped_lock<interprocess_mutex> lock(pd.hdr->mutex);
	xmprintf(3, "cmd = %d  \n", pd.hdr->cmd);
	pd.hdr->cmd = CmdHeader::exit;
	pd.hdr->cmdWait.notify_all();
	xmprintf(3, "TEST: start waiting ..\n");
	pd.hdr->workDone.wait(lock);
	status = 4; //   stopped
}

void SHMTest::qwtshowmw() {
	if (status != 0) return;
	sendCommand(CmdHeader::qMW);
}
void SHMTest::qwttitle(const char* s) {
	if (status != 0) return;
	sendCommand(CmdHeader::qTitle, s);
}
void SHMTest::qwtxlabel(const char* s) {
	if (status != 0) return;
	sendCommand(CmdHeader::qXlabel, s);
}
void SHMTest::qwtylabel(const char* s) {
	if (status != 0) return;
	sendCommand(CmdHeader::qYlabel, s);
}
void SHMTest::qwtclear() {
	if (status != 0) return;
	sendCommand(CmdHeader::qClear);
}

void SHMTest::qsetloglevel(int level) {
	if (status != 0) return;
	sendCommand(CmdHeader::qSetLogLevel, level);
}

void SHMTest::sendCommand(CmdHeader::QWCmd cmd, const char* text) {
	if (status != 0) return;
	using namespace boost::interprocess;
	scoped_lock<interprocess_mutex> lock(pd.hdr->mutex);
	pd.hdr->cmd = cmd;
	if (text != 0) {
		strncpy(pd.hdr->name, text, CmdHeader::nameSize);
	}
	pd.hdr->cmdWait.notify_all();
	xmprintf(4, "TEST: start waiting ..\n");
	pd.hdr->workDone.wait(lock);
}
void SHMTest::sendCommand(CmdHeader::QWCmd cmd, int v) {
	if (status != 0) return;
	using namespace boost::interprocess;
	scoped_lock<interprocess_mutex> lock(pd.hdr->mutex);
	pd.hdr->cmd = cmd;
	pd.hdr->test = v;

	pd.hdr->cmdWait.notify_all();
	xmprintf(4, "TEST: start waiting ..\n");
	pd.hdr->workDone.wait(lock);
}

#ifdef ENABLE_UDP_SYNC
	void SHMTest::qwtEnableCoordBroadcast(double* x, double* y, double* z, double* time, int size) {
		if (status != 0) return;
		using namespace boost::interprocess;
		scoped_lock<interprocess_mutex> lock(pd.hdr->mutex);

		pd.hdr->cmd = CmdHeader::qEnableBC;
		pd.hdr->size = size;
		pd.x = x;
		pd.y = y;
		pd.z = z;

		pd.hdr->cmdWait.notify_all();
		pd.hdr->workDone.wait(lock);	
	}
	
	void SHMTest::qwtDisableCoordBroadcast() {
		if (status != 0) return;
		using namespace boost::interprocess;
		scoped_lock<interprocess_mutex> lock(pd.hdr->mutex);
		pd.hdr->cmd = CmdHeader::qDisableBC;
		pd.hdr->cmdWait.notify_all();
		pd.hdr->workDone.wait(lock);	
	}
#endif


void SHMTest::qwtplot(double* x, double* y, int size, const char* name, const char* style, 
    	int lineWidth, int symSize) {
	qwtplot2(x, y, size, name, style, lineWidth, symSize, 0);
}

void SHMTest::qwtplot2(double* x, double* y, int size, const char* name, const char* style, 
    	int lineWidth, int symSize, double* time) {
	if (status != 0) return;
	using namespace boost::interprocess;
	scoped_lock<interprocess_mutex> lock(pd.hdr->mutex);
	
	//   check max size on the other side:
	long long a = pd.hdr->segSize;
	if (a < size) {
		xmprintf(3, "SHMTest: inc seg size (1); current size = %lld \n", a);
		pd.hdr->cmd = CmdHeader::changeSize;
		pd.hdr->size = size;	
		pd.hdr->cmdWait.notify_all();
		pd.hdr->workDone.wait(lock);

		//  now we have to adjust our memory somehow..
		long long segSize = pd.hdr->segSize;
		xmprintf(3, "SHMTest: new size is %lld \n", segSize);
		
		// truncate our part to the new size
		shmX.truncate(segSize * sizeof(double));
		shmY.truncate(segSize * sizeof(double));
		shmZ.truncate(segSize * sizeof(double));

		//  reassign the mapping regions
		xReg = mapped_region(shmX, read_write);
		yReg = mapped_region(shmY, read_write);
		tReg = mapped_region(shmZ, read_write);

		//  update addresses
		pd.x = static_cast<double*>(xReg.get_address());
		pd.y = static_cast<double*>(yReg.get_address());
		pd.t = static_cast<double*>(tReg.get_address());
	}

	//  now lets plot
	pd.hdr->lineWidth = lineWidth;
	pd.hdr->symSize = symSize;
	strncpy(pd.hdr->style, style, 8);
	pd.hdr->size = size;
	strncpy(pd.hdr->name, name, CmdHeader::nameSize);

	memcpy(pd.x, x, sizeof(double) * size);
	memcpy(pd.y, y, sizeof(double) * size);

	if (time != 0) {
		pd.hdr->cmd = CmdHeader::qPlot2;
		memcpy(pd.t, time, sizeof(double) * size);
	} else {
		pd.hdr->cmd = CmdHeader::qPlot;
	}
	
	pd.hdr->cmdWait.notify_all();
	pd.hdr->workDone.wait(lock);
}


