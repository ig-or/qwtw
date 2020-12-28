
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

#ifdef USEMARBLE
void SHMTest::qwtmap(int n) {
	if (status != 0) return;
	sendCommand(CmdHeader::qMap, n);
}
#endif


void SHMTest::qwtfigure(int n) {
	if (status != 0) return;
	sendCommand(CmdHeader::qFigure, n);
}
void SHMTest::qwtsetimpstatus(int impStatus) {
	if (status != 0) return;
	sendCommand(CmdHeader::qImpStatus, impStatus);
}
#ifdef USEMARBLE
int SHMTest::startProc(const std::string& mdp, const std::string& mpp, int level) {
#else
int SHMTest::startProc(int level) {
#endif
	xmprintf(2, "\nstarting proc.. \n");
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
	std::string qwProcPath;
	qwProcPath.clear();

	// current path?
	try {
		pa = current_path() / procName;
		xmprintf(2, "\tlooking at %s .. \n", pa.string().c_str());
		if (exists(pa)) {
			qwProcPath = pa.string();
			xmprintf(2, "\tlocated!\n");
		}
	} catch (std::exception& ex) {
		xmprintf(2, "exception: %s\n", ex.what());
		qwProcPath.clear();
	}
#ifdef WIN32

#else
	if (qwProcPath.empty()) { //   near current EXE?
		char result[ PATH_MAX ];
		ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
		pa = path(result).parent_path() / procName;
		xmprintf(2, "\tlookigng at %s \n", pa.string().c_str());
		if (exists(pa)) {
			qwProcPath = pa.string();
			xmprintf(2, "\tgot exe in %s\n", qwProcPath.c_str());
		} else {
			xmprintf(2, "\t\t ..  nothing found\n");
		}
	}
#endif

	if (qwProcPath.empty()) {
		xmprintf(2, "\tnow lookign in PATH; \n");
		try {
			pa = bp::search_path(procName);
			qwProcPath = pa.string();
			xmprintf(2, "got qwproc in [%s] \n", qwProcPath.c_str());
		} catch  (std::exception& ex) {
			xmprintf(0, "cannot find %s in PATH (1) \n", procName);
			return 2;
		}
	}

	if (qwProcPath.empty()) {
		xmprintf(0, "cannot find %s in PATH (2) \n", procName);
		return 2;
	}

	//std::this_thread::sleep_for(10ms);
	try {
		std::string sLevel = std::to_string(level);
#ifdef USEMARBLE
		xmprintf(2, "starting %s %s \n", qwProcPath.c_str(), mdp.c_str());
		if (mdp.empty()) {
			if (mpp.empty()) {
				bp::spawn(qwProcPath, "--debug", sLevel.c_str());
			} else {
				bp::spawn(qwProcPath, "--marble_plugins", mpp.c_str(), "--debug", sLevel.c_str());
			}
		} else {
			if (mpp.empty()) {
				bp::spawn(qwProcPath, "--marble_data", mdp.c_str(), "--debug", sLevel.c_str());
			}else {
				bp::spawn(qwProcPath, "--marble_data", mdp.c_str(), "--marble_plugins", mpp.c_str(), "--debug", sLevel.c_str());
			}
		}
#else
		bp::spawn(qwProcPath, "--debug", sLevel.c_str());
#endif
		std::this_thread::sleep_for(275ms);
		xmprintf(3, "qwproc supposed to start from  (%s) \n", qwProcPath.c_str());
	}	catch (std::exception& ex) {
		xmprintf(0, "cannot start process %s from (%s) (%s) \n", procName, qwProcPath.c_str(), ex.what());
		return 1;
	}
	xmprintf(2, "SHMTest::startProc() exiting \n");
	return 0;
}
#ifdef USEMARBLE
int SHMTest::testInit(const std::string& mdp, const std::string& mpp, int level) {
#else
int SHMTest::testInit(int level) {
#endif
	xmprintf(2, "starting SHMTest::testInit()\n");
	if (status == 0) {
		//return 0;
		xmprintf(2, "starting SHMTest::testInit()  (status == 0)\n");
	}
	using namespace boost::interprocess;
	
	int test = checkProcRunning();
	if (test == 0) {  //  not running
#ifdef USEMARBLE
		startProc(mdp, mpp, level);
#else
		startProc(level);
#endif
		
		//  try one more time
		test = checkProcRunning();
		if (test == 0) {  // still not running
			status = 1;
			xmprintf(1, "ERROR: cannot start qwproc\n");
			return 1; //  cannot start the program  (not installed?)
		}
	}

	xmprintf(3, "SHMTest::testInit() setting up memory\n");
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

	status = 0;   ///  checked inside qsetloglevel
	xmprintf(2, "starting SHMTest::testInit() making qsetloglevel..\n");
	qsetloglevel(level);

	xmprintf(2, "starting SHMTest::testInit() finished\n");
	return 0;
}

void SHMTest::stopQt() {
	if (status != 0) return;
	using namespace boost::interprocess;
	xmprintf(3, "SHMTest::stopQt();  locking.. \n");
	scoped_lock<interprocess_mutex> lock(pd.hdr->mutex);
	xmprintf(3, "\tSHMTest::stopQt();  locked \n");
	pd.hdr->cmd = CmdHeader::exit;
	pd.hdr->cmdWait.notify_all();
	xmprintf(3, "\tSHMTest::stopQt();  start waiting ..\n");
	pd.hdr->workDone.wait(lock);
	status = 4; //   stopped
	xmprintf(3, "\tSHMTest::stopQt();  done\n");
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
	//int tmp = xmPrintLevel;
	//xmPrintLevel = 10;
	sendCommand(CmdHeader::qSetLogLevel, level);
	//xmPrintLevel = tmp; // level
}

void SHMTest::sendCommand(CmdHeader::QWCmd cmd, const char* text) {
	if (status != 0) return;
	using namespace boost::interprocess;
	xmprintf(4, "SHMTest::sendCommand(%d, %s): locking ..\n", static_cast<int>(cmd), text);
	scoped_lock<interprocess_mutex> lock(pd.hdr->mutex);
	xmprintf(4, "\tSHMTest::sendCommand(%d, %s): locked ..\n", static_cast<int>(cmd), text);
	pd.hdr->cmd = cmd;
	if (text != 0) {
		strncpy(pd.hdr->name, text, CmdHeader::nameSize);
	}
	pd.hdr->cmdWait.notify_all();
	xmprintf(4, "\tSHMTest::sendCommand(%d, %s): start waiting ..\n", static_cast<int>(cmd), text);
	pd.hdr->workDone.wait(lock);
	xmprintf(4, "\tSHMTest::sendCommand(%d, %s): complete\n", static_cast<int>(cmd), text);
}
void SHMTest::sendCommand(CmdHeader::QWCmd cmd, int v) {
	if (status != 0) return;
	using namespace boost::interprocess;
	xmprintf(4, "SHMTest::sendCommand(%d, %d): locking ..\n", static_cast<int>(cmd), v);
	scoped_lock<interprocess_mutex> lock(pd.hdr->mutex);
	xmprintf(4, "\tSHMTest::sendCommand locked. \n");
	pd.hdr->cmd = cmd;
	pd.hdr->test = v;

	pd.hdr->cmdWait.notify_all();
	xmprintf(4, "\tSHMTest::sendCommand(%d, %d): start waiting ..\n", static_cast<int>(cmd), v);
	pd.hdr->workDone.wait(lock);
	xmprintf(4, "\tSHMTest::sendCommand(%d, %d): finished\n", static_cast<int>(cmd), v);
}

#ifdef ENABLE_UDP_SYNC
	void SHMTest::qwtEnableCoordBroadcast(double* x, double* y, double* z, double* time, int size) {
		if (status != 0) return;
		using namespace boost::interprocess;

		pd.hdr->mutex.lock();
		long long a = pd.hdr->segSize;
		pd.hdr->mutex.unlock();
		if (a < size) {
			resize(size);
		}

		xmprintf(3, "SHMTest::qwtEnableCoordBroadcast();  locking ..\n");
		scoped_lock<interprocess_mutex> lock(pd.hdr->mutex);
		xmprintf(3, "\tSHMTest::qwtEnableCoordBroadcast();  locked ..\n");

		pd.hdr->cmd = CmdHeader::qEnableBC;
		pd.hdr->size = size;

		memcpy(pd.x, x, sizeof(double) * size);
		memcpy(pd.y, y, sizeof(double) * size);
		memcpy(pd.z, z, sizeof(double) * size);
		memcpy(pd.t, time, sizeof(double) * size);


		pd.hdr->cmdWait.notify_all();
		xmprintf(3, "\tSHMTest::qwtEnableCoordBroadcast();  waiting ..\n");
		pd.hdr->workDone.wait(lock);	
		xmprintf(3, "\tSHMTest::qwtEnableCoordBroadcast();  finished ..\n");
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

void SHMTest::resize(long long size) {
	using namespace boost::interprocess;
	xmprintf(3, "SHMTest::resize(); size = %d  locking ..\n", size);
	scoped_lock<interprocess_mutex> lock(pd.hdr->mutex);
	xmprintf(3, "\tSHMTest::resize();  locked ..\n");

	pd.hdr->cmd = CmdHeader::changeSize;
	pd.hdr->size = size;
	pd.hdr->cmdWait.notify_all();
	xmprintf(4, "\tSHMTest::resize: inc seg size, locking.. \n");
	pd.hdr->workDone.wait(lock);
	xmprintf(4, "\tSHMTest::resize: inc seg size, locked. \n");

	//  now we have to adjust our memory somehow..
	long long segSize = pd.hdr->segSize;
	xmprintf(3, "\tSHMTest::resize: new size is %lld \n", segSize);

	// truncate our part to the new size
	shmX.truncate(segSize * sizeof(double));
	shmY.truncate(segSize * sizeof(double));
	shmZ.truncate(segSize * sizeof(double));
	shmT.truncate(segSize * sizeof(double));

	//  reassign the mapping regions
	xReg = mapped_region(shmX, read_write);
	yReg = mapped_region(shmY, read_write);
	zReg = mapped_region(shmZ, read_write);
	tReg = mapped_region(shmT, read_write);

	//  update addresses
	pd.x = static_cast<double*>(xReg.get_address());
	pd.y = static_cast<double*>(yReg.get_address());
	pd.z = static_cast<double*>(zReg.get_address());
	pd.t = static_cast<double*>(tReg.get_address());
	xmprintf(6, "\tSHMTest::qwtplot2: new size end \n");
}

void SHMTest::qwtplot2(double* x, double* y, int size, const char* name, const char* style, 
    	int lineWidth, int symSize, double* time) {
	if (status != 0) return;
	using namespace boost::interprocess;
	
	//   check max size on the other side:
	pd.hdr->mutex.lock();
	long long a = pd.hdr->segSize;
	pd.hdr->mutex.unlock();
	if (a < size) {
		xmprintf(3, "\tSHMTest::qwtplot2: inc seg size (1); current size = %lld \n", a);
		resize(size);
	}
	xmprintf(3, "SHMTest::qwtplot2(); size = %d  locking ..\n", size);
	scoped_lock<interprocess_mutex> lock(pd.hdr->mutex);
	xmprintf(3, "\tSHMTest::qwtplot2();  locked ..\n");

	//  now lets plot
	xmprintf(6, "\tSHMTest::qwtplot2: copying .. \n");
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
	xmprintf(3, "\tSHMTest::qwtplot2(); notifying..\n");
	pd.hdr->cmdWait.notify_all();
	xmprintf(3, "\tSHMTest::qwtplot2();  waiting ..\n");
	pd.hdr->workDone.wait(lock);
	xmprintf(3, "\tSHMTest::qwtplot2();  done\n");
}


