
#include "xstdef.h"
#include "pclient.h"

#include "qwtypes.h"

#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/process/spawn.hpp>
#include <boost/process/search_path.hpp>
#include <thread>
#include <chrono>
#include <cstdlib>

SHMTest::SHMTest(): status(5) {

}

#ifdef USEMARBLE
int SHMTest::qwtmap(int n) {
	if (status != 0) return 0;
	int test = sendCommand(CmdHeader::qMap, n);
	return test;
}
#endif


int SHMTest::qwtfigure(int n) {
	if (status != 0) return 0;
	int test = sendCommand(CmdHeader::qFigure, n);
	return test;
}
void SHMTest::qwtClipGroup(int gr) {
	if (status != 0) return;
	sendCommand(CmdHeader::qSetClipGroup, gr);
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
		xmprintf(2, "starting %s %s %s\n", qwProcPath.c_str(), mdp.c_str(), mpp.c_str());
		if (mdp.empty()) {
			if (mpp.empty()) {
				bp::spawn(qwProcPath, "--debug", sLevel.c_str());
				xmprintf(2, "@starting %s --debug %s\n", qwProcPath.c_str(), sLevel.c_str());
			} else {
				bp::spawn(qwProcPath, "--marble_plugins", mpp.c_str(), "--debug", sLevel.c_str());
				xmprintf(2, "@starting %s --marble_plugins %s --debug %s\n", 
					qwProcPath.c_str(), mpp.c_str(), sLevel.c_str());
			}
		} else {
			if (mpp.empty()) {
				bp::spawn(qwProcPath, "--marble_data", mdp.c_str(), "--debug", sLevel.c_str());
				xmprintf(2, "@starting %s --marble_data %s  --debug %s\n", 
					qwProcPath.c_str(), mdp.c_str(), sLevel.c_str());

			}else {
				bp::spawn(qwProcPath, "--marble_data", mdp.c_str(), "--marble_plugins", mpp.c_str(), "--debug", sLevel.c_str());
				xmprintf(2, "@starting %s --marble_data %s --marble_plugins %s --debug %s\n", 
					qwProcPath.c_str(), mdp.c_str(), mpp.c_str(), sLevel.c_str());
			}
		}
#else
		bp::spawn(qwProcPath, "--debug", sLevel.c_str());
		xmprintf(2, "@starting %s --debug %s\n", qwProcPath.c_str(), sLevel.c_str());

#endif
		std::this_thread::sleep_for(475ms);
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
	using namespace std::chrono_literals;	
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
		for (int a = 0; a < 5; a++) {
			test = checkProcRunning();
			if (test == 0) {  // still not running
				std::this_thread::sleep_for(375ms);
			} else {
				xmprintf(3, "\tSHMTest::testInit() checkProcRunning() test = %d \n", test);
				break;
			}
		}
		if (test == 0) {  // still not running
			status = 1;
			xmprintf(0, "ERROR: cannot start qwproc\n");
			return 1; //  cannot start the program  (not installed?)
		}
	}
	xmprintf(3, "\nSHMTest::testInit() looks like qwtwc is (was?) running \n");
	xmprintf(3, "\tSHMTest::testInit() setting up memory\n");
	int attemptCount = 10;
	bool shmConnected = false;
	std::string exPlanation;
	while (attemptCount > 0) {
		try {
			shared_memory_object shmCommand_(open_only, ProcData::shmNames[0], read_write);
			shmCommand.swap(shmCommand_);
			shmConnected = true;
			xmprintf(3, "\tSHMTest::testInit() connected to SHM; attemptCount = %d\n", attemptCount);
			break;
		} catch(interprocess_exception &ex) { // proc not started?  something is not OK
			exPlanation.assign(ex.what());
			xmprintf(3, "\tSHMTest::testInit() proc not started? (attemptCount = %d)  something is not OK; (%s)\n", 
				attemptCount, exPlanation.c_str());
			std::this_thread::sleep_for(275ms);
		}
		attemptCount -= 1;
	}
	if (!shmConnected) {
			xmprintf(0, "SHMTest::testInit():  proc not started? cannot connect to the SHM (%s) \n", exPlanation.c_str());
			status = 2;
			return 2;
	}
	std::this_thread::sleep_for(27ms);

	xmprintf(4, "creating shared_memory_object's.... \n");
	shared_memory_object shmX_(open_only, ProcData::shmNames[1], read_write);
	shared_memory_object shmY_(open_only, ProcData::shmNames[2], read_write);
	shared_memory_object shmZ_(open_only, ProcData::shmNames[3], read_write);
	shared_memory_object shmT_(open_only, ProcData::shmNames[4], read_write);
	shared_memory_object shmData_(open_only, ProcData::shmNames[5], read_write);
	
	shmX.swap(shmX_);
	shmY.swap(shmY_);
	shmZ.swap(shmZ_);
	shmT.swap(shmT_);
	shmData.swap(shmData_);

	shmCommand.truncate(sizeof(CmdHeader));
	mapped_region commandReg_ = mapped_region(shmCommand, read_write);
	commandReg.swap(commandReg_);
	xmprintf(4, "shared_memory_object's created \n");
	pd.hdr = static_cast<CmdHeader*>(commandReg.get_address());
	{
		xmprintf(4, "trying to adjust the memory according to header info ... \n");
		try {
			scoped_lock<interprocess_mutex> lock(pd.hdr->mutex);
		} catch (interprocess_exception &ex) { 
			xmprintf(0, "ERROR SHMTest::testInit() (124): (%s) \n", ex.what());
			return 8;
		}
		long long segSize = pd.hdr->segSize;
		long long dataSize = pd.hdr->dataSize;
		shmX.truncate(segSize * sizeof(double));
		shmY.truncate(segSize * sizeof(double));
		shmZ.truncate(segSize * sizeof(double));
		shmT.truncate(segSize * sizeof(double));
		shmData.truncate(dataSize * sizeof(double));
		xmprintf(4, "memory adjusted \n");
	}

	xmprintf(4, "creating local memory object pointers.. \n");
	mapped_region xReg_ = mapped_region(shmX, read_write);
	mapped_region yReg_ = mapped_region(shmY, read_write);
	mapped_region zReg_ = mapped_region(shmZ, read_write);
	mapped_region tReg_ = mapped_region(shmT, read_write);
	mapped_region dataReg_ = mapped_region(shmData, read_write);

	xReg.swap(xReg_);
	yReg.swap(yReg_);
	zReg.swap(zReg_);
	tReg.swap(tReg_);
	dataReg.swap(dataReg_);

	xmprintf(4, "local pointers created \n");

	pd.x = static_cast<double*>(xReg.get_address());
	pd.y = static_cast<double*>(yReg.get_address());
	pd.z = static_cast<double*>(zReg.get_address());
	pd.t = static_cast<double*>(tReg.get_address());
	pd.data = static_cast<double*>(dataReg.get_address());

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

int SHMTest::sendCommand(CmdHeader::QWCmd cmd, const char* text) {
	if (status != 0) return 0;
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

	int test = pd.hdr->test;
	xmprintf(4, "\tSHMTest::sendCommand(%d, %s): test = %d\n", static_cast<int>(cmd), text, test);
	return test;

}
int SHMTest::sendCommand(CmdHeader::QWCmd cmd, int v) {
	if (status != 0) return 0;
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

	int test = pd.hdr->test;
	xmprintf(4, "\tSHMTest::sendCommand(%d, %d): test = %d\n", static_cast<int>(cmd), v, test);
	return test;
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


int SHMTest::qwtplot(double* x, double* y, int size, const char* name, const char* style, 
    	int lineWidth, int symSize) {
	return qwtplot2(x, y, size, name, style, lineWidth, symSize, 0);
}

void SHMTest::resizeData(long long size) {
	using namespace boost::interprocess;
	xmprintf(3, "SHMTest::resizeData(); size = %d  locking ..\n", size);
	scoped_lock<interprocess_mutex> lock(pd.hdr->mutex);
	xmprintf(3, "\tSHMTest::resizeData();  locked ..\n");

	pd.hdr->cmd = CmdHeader::changeDataSize;
	pd.hdr->size = size;
	pd.hdr->cmdWait.notify_all();
	xmprintf(4, "\tSHMTest::resizeData: inc seg size, locking.. \n");
	pd.hdr->workDone.wait(lock);
	xmprintf(4, "\tSHMTest::resizeData: inc seg size, locked. \n");

	//  now we have to adjust our memory somehow..
	long long dataSize = pd.hdr->dataSize;
	xmprintf(3, "\tSHMTest::resizeData: new size is %lld \n", dataSize);

	// truncate our part to the new size
	shmData.truncate(dataSize * sizeof(double));

	//  reassign the mapping regions
	dataReg = mapped_region(shmData, read_write);

	//  update addresses
	pd.data = static_cast<double*>(dataReg.get_address());

	xmprintf(6, "\tSHMTest::resizeData: new size end \n");
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
	xmprintf(6, "\tSHMTest::resize: new size end \n");
}

int SHMTest::qwtplot2(double* x, double* y, int size, const char* name, const char* style, 
    	int lineWidth, int symSize, double* time) {
	if (status != 0) return -7;
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
	try {
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
	} catch (const std::exception& ex){
		xmprintf(0, "ERROR: exception in SHMTest::qwtplot2 (%s)\n", ex.what());
		return -8;
	}
	xmprintf(3, "\tSHMTest::qwtplot2(); notifying..\n");
	pd.hdr->cmdWait.notify_all();
	xmprintf(3, "\tSHMTest::qwtplot2();  waiting ..\n");
	pd.hdr->workDone.wait(lock);
	int test = pd.hdr->test;
	xmprintf(3, "\tSHMTest::qwtplot2();  done\n");
	return test;
}

void SHMTest::qwtremove(int id) {
	if (status != 0) return;
	sendCommand(CmdHeader::qRemoveLine, id);
}

int SHMTest::qwtchange(int id, double* x, double* y, double* z, double* time, int size) {
	if (status != 0) return -7;
	using namespace boost::interprocess;

	//   check max size on the other side:
	pd.hdr->mutex.lock();
	long long a = pd.hdr->segSize;
	pd.hdr->mutex.unlock();
	if (a < size) {
		xmprintf(3, "\tSHMTest::qwtchange: inc seg size (1); current size = %lld \n", a);
		resize(size);
	}
	xmprintf(3, "SHMTest::qwtchange(); size = %d  locking ..\n", size);
	scoped_lock<interprocess_mutex> lock(pd.hdr->mutex);
	xmprintf(3, "\tSHMTest::qwtchange();  locked ..\n");

	//  now lets change
	xmprintf(6, "\tSHMTest::qwtchange: copying the data \n");
	try {
		//pd.hdr->lineWidth = lineWidth;
		//pd.hdr->symSize = symSize;
		//strncpy(pd.hdr->style, style, 8);
		pd.hdr->size = size;
		//strncpy(pd.hdr->name, name, CmdHeader::nameSize);

		memcpy(pd.x, x, sizeof(double) * size);
		memcpy(pd.y, y, sizeof(double) * size);
		pd.hdr->cmd = CmdHeader::qChangeLine;
		pd.hdr->test = id;

		if (time != 0) {
			pd.hdr->timeIsGood = 1;
			memcpy(pd.t, time, sizeof(double) * size);
		}	else {
			pd.hdr->timeIsGood = 0;
		}
		if (z != 0) {
			pd.hdr->zIsGood = 1;
			memcpy(pd.z, z, sizeof(double) * size);
		}	else {
			pd.hdr->zIsGood = 0;
		}
		
	}
	catch (const std::exception& ex) {
		xmprintf(0, "ERROR: exception in SHMTest::qwtchange (%s)\n", ex.what());
		return -8;
	}
	xmprintf(3, "\tSHMTest::qwtchange(); notifying..\n");
	pd.hdr->cmdWait.notify_all();
	xmprintf(3, "\tSHMTest::qwtchange();  waiting ..\n");
	pd.hdr->workDone.wait(lock);
	int test = pd.hdr->test;
	xmprintf(3, "\tSHMTest::qwtchange();  done\n");
	return test;
}


#ifdef USEMATHGL
int SHMTest::qwtmgl(int n) {
	if (status != 0) return 0;
	int test = sendCommand(CmdHeader::qMglPlot, n);
	return test;
}

void SHMTest::qwtmgl_line(int size, double* x, double* y, double* z, const char* name, const char* style) {
	if (status != 0) return;
	using namespace boost::interprocess;
	
	//   check max size on the other side:
	pd.hdr->mutex.lock();
	long long a = pd.hdr->segSize;
	pd.hdr->mutex.unlock();
	if (a < size) {
		xmprintf(3, "\tSHMTest::qwtmgl_line: inc seg size (1); current size = %lld \n", a);
		resize(size);
	}
	xmprintf(3, "SHMTest::qwtmgl_line(); size = %d  locking ..\n", size);
	scoped_lock<interprocess_mutex> lock(pd.hdr->mutex);
	xmprintf(3, "\tSHMTest::qwtmgl_line();  locked ..\n");

	//  now lets plot
	xmprintf(6, "\tSHMTest::qwtmgl_line: copying .. \n");
	strncpy(pd.hdr->style, style, 32);
	pd.hdr->size = size;
	strncpy(pd.hdr->name, name, CmdHeader::nameSize);

	memcpy(pd.x, x, sizeof(double) * size);
	memcpy(pd.y, y, sizeof(double) * size);
	memcpy(pd.z, z, sizeof(double) * size);

/*
	if (time != 0) {
		pd.hdr->cmd = CmdHeader::qPlot2;
		memcpy(pd.t, time, sizeof(double) * size);
	} else {
		pd.hdr->cmd = CmdHeader::qPlot;
	}
	*/
	pd.hdr->cmd = CmdHeader::qMglLine;

	xmprintf(3, "\tSHMTest::qwtmgl_line(); notifying..\n");
	pd.hdr->cmdWait.notify_all();
	xmprintf(3, "\tSHMTest::qwtmgl_line();  waiting ..\n");
	pd.hdr->workDone.wait(lock);
	xmprintf(3, "\tSHMTest::qwtmgl_line();  done\n");
}

void SHMTest::qwtmgl_mesh(const MeshInfo& info)  {

	if (status != 0) return;
	using namespace boost::interprocess;
	
	//   check max size on the other side:
	pd.hdr->mutex.lock();

	long long a = pd.hdr->dataSize;
	pd.hdr->mutex.unlock();
	long long size = info.xSize * info.ySize;
	if (a < size) {
		xmprintf(3, "\tSHMTest::qwtmgl_mesh: inc seg size (1); current size = %lld \n", a);
		resizeData(size);
	}
	xmprintf(3, "SHMTest::qwtmgl_mesh(); size = %d  locking ..\n", size);
	scoped_lock<interprocess_mutex> lock(pd.hdr->mutex);
	xmprintf(3, "\tSHMTest::qwtmgl_mesh();  locked ..\n");

	//  now lets plot
	xmprintf(6, "\tSHMTest::qwtmgl_mesh: copying .. \n");
	strncpy(pd.hdr->style, info.style.c_str(), 32);
	strncpy(pd.hdr->name, info.name.c_str(), CmdHeader::nameSize);

	pd.hdr->size = size;
	pd.hdr->xSize = info.xSize;
	pd.hdr->ySize = info.ySize;
	pd.hdr->xMin = info.xMin;
	pd.hdr->xMax = info.xMax;
	pd.hdr->yMin = info.yMin;
	pd.hdr->yMax = info.yMax;
	pd.hdr->type = static_cast<int>(info.sd);
	
	memcpy(pd.data, info.data, sizeof(double) * size);
	

	pd.hdr->cmd = CmdHeader::qMglMesh;

	xmprintf(3, "\tSHMTest::qwtmgl_mesh(); notifying..\n");
	pd.hdr->cmdWait.notify_all();
	xmprintf(3, "\tSHMTest::qwtmgl_mesh();  waiting ..\n");
	pd.hdr->workDone.wait(lock);
	xmprintf(3, "\tSHMTest::qwtmgl_mesh();  done\n");
}

#endif
