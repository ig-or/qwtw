
#include "xstdef.h"
#include "pclient.h"

#include "qwtypes.h"

#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/process/spawn.hpp>
#include <boost/process/search_path.hpp>
#include <boost/chrono.hpp>
#include <boost/thread/thread_time.hpp>
#include <thread>
#include <chrono>
#include <cstdlib>
#ifdef WIN32
//	#include <windows.h>  //  cannot use it here
const char* getQwtwDllPath();
#endif

SHMTest::SHMTest(): status(5) {
	
}

SHMTest::~SHMTest() {
//#ifndef WIN32   // in Windows it sometimes hangs on boost::interprocess::interprocess_condition::notify_all
	onClose();
//#endif
}
#ifdef USEMARBLE
int SHMTest::qwtmap(int n) {
	if (status != 0) return 0;
	int test = sendCommand(CmdHeader::qMap, n);
	return test;
}
#endif
int SHMTest::qwtservice(int x) {
	if (status != 0) return 0;
	int test = sendCommand(CmdHeader::qService, x);
	return test;
}

int SHMTest::qwtfigure(int n, unsigned int flags) {
	if (status != 0) return 0;
	int test = sendCommand(CmdHeader::qFigure, n, flags);
	return test;
}
int SHMTest::qwtspectrogram(int n, unsigned int flags) {
	if (status != 0) return 0;
	int test = sendCommand(CmdHeader::qSpectrogram, n, flags);
	return test;
}
int SHMTest::spectrogram_info(const SpectrogramInfo& info) {
	if (status != 0) return 0;
	using namespace boost::interprocess;

	//   check max size on the other side:
	pd.hdr->mutex.lock();
	long long a = pd.hdr->dataSize;
	pd.hdr->mutex.unlock();
	long long size = info.nx * info.ny;
	if (a < size) {
		xmprintf(3, "\tSHMTest::spectrogram_info: inc seg size (1); current size = %lld \n", a);
		resizeData(size);
	}

	// do we have additional info? 
	bool haveExtInfo = false;
	if ((info.p != 0) || (info.t != 0)) {
		haveExtInfo = true;
	}
	if (haveExtInfo) {  //  if yes, check the size
		pd.hdr->mutex.lock();
		long long a = pd.hdr->segSize;
		pd.hdr->mutex.unlock();

		if (a < size*3) {
			xmprintf(3, "\tSHMTest::spectrogram_info: inc seg size (2); current size = %lld \n", a);
			resize(size*3);
		}
	}

	xmprintf(3, "SHMTest::spectrogram_info(); size = %d;  locking ..\n", size);
	scoped_lock<interprocess_mutex> lock(pd.hdr->mutex);
	xmprintf(3, "\tSHMTest::spectrogram_info();  locked ..\n");

	//  now lets plot
	xmprintf(6, "\tSHMTest::spectrogram_info: copying .. \n");
	pd.hdr->size = size;
	pd.hdr->xSize = info.nx;
	pd.hdr->ySize = info.ny;
	pd.hdr->xMin = info.xmin;
	pd.hdr->xMax = info.xmax;
	pd.hdr->yMin = info.ymin;
	pd.hdr->yMax = info.ymax;
	memcpy(pd.data, info.z, sizeof(double) * size);

	pd.hdr->flags = 0;
	if (haveExtInfo) {
		if (info.t != 0) {
			pd.hdr->flags |= 1;
			memcpy(pd.t, info.t, sizeof(double) * size);
		}
		if ((info.p != 0)) {
			pd.hdr->flags |= 2;
			memcpy(pd.x, info.p, sizeof(double) * size * 3);
		}
	}

	pd.hdr->cmd = CmdHeader::qSpectrogramInfo;
	//  at this point, all the info is copied

	xmprintf(3, "\tSHMTest::spectrogram_info(); notifying..\n");
	pd.hdr->cmdWait.notify_all();
	xmprintf(3, "\tSHMTest::spectrogram_info();  waiting ..\n");
	pd.hdr->workDone.wait(lock);
	xmprintf(3, "\tSHMTest::spectrogram_info();  done\n");

	return 0;
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
#ifdef WIN32      //   check the location near the DLL
	try {
		const char* dllPath = getQwtwDllPath();
		pa = path(dllPath).parent_path() / procName;
		xmprintf(2, "\tlooking at %s .. \n", pa.string().c_str());
		if (exists(pa)) {
			qwProcPath = pa.string();
			xmprintf(2, "\tlocated!\n");
		}
	} catch (std::exception& ex) {
		xmprintf(2, "exception: %s\n", ex.what());
		qwProcPath.clear();
	}
	
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
	if (cmdSync == nullptr) {
		cmdSync = std::make_unique<CmdSync>();
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
			xmprintf(5, "\t entering lock on  pd.hdr->mutex..\n");
			//pd.hdr->mutex.lock();
			bool lock_test = pd.hdr->mutex.try_lock_for(250ms);
			if (lock_test) {
				xmprintf(8, " \t locked! (try_lock_for = true) \n");
			} else {
				pd.hdr->mutex.unlock();
				xmprintf(0, "ERROR SHMTest::testInit() (128): cannot lock mutex; qwproc hangs?\n");
				return 14;
			}
			//scoped_lock<interprocess_mutex> lock(pd.hdr->mutex);
		} catch (interprocess_exception &ex) { 
			pd.hdr->mutex.unlock();
			xmprintf(0, "ERROR SHMTest::testInit() (124): (%s) \n", ex.what());
			return 8;
		}
		xmprintf(5, "\t locked!\n");
		long long segSize = pd.hdr->segSize;
		long long dataSize = pd.hdr->dataSize;
		shmX.truncate(segSize * sizeof(double));
		shmY.truncate(segSize * sizeof(double));
		shmZ.truncate(segSize * sizeof(double));
		shmT.truncate(segSize * sizeof(double));
		shmData.truncate(dataSize * sizeof(double));
		pd.hdr->mutex.unlock();
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

	//  start the callback thread:
	needStopCallbackThread = false;
	if (!cbThread.joinable()) { //   running already??	
		std::thread ttmp(&SHMTest::cbThreadF, this);
		cbThread.swap(ttmp);
	}
	if (!cbThread_2.joinable()) {
		std::thread ttmp2(&SHMTest::cbThreadF_2, this);
		cbThread_2.swap(ttmp2);
	}

	xmprintf(2, "starting SHMTest::testInit() finished\n");
	return 0;
}

void SHMTest::onClose() {
	//  stop two callback threads
	xmprintf(9, "SHMTest::onClose() starting  \n");
	needStopCallbackThread = true;
	if (cbThread.joinable()) { //   stop callback thread?
		//xmprintf(8, "\t stopping cbThread  \n");

		//cmdSync->cbWait.notify_all();  //  this may hang forever

		cbThread.join();
		//xmprintf(8, "\t cbThread finished \n");
	}
	if (cbThread_2.joinable()) {
		//cbiReady.notify_all();
		//xmprintf(8, "\t stopping cbThread_2  \n");
		cbThread_2.join();
		//xmprintf(8, "\t cbThread_2 finished \n");
	}

	if (cmdSync != nullptr) {
		cmdSync.reset();
		cmdSync = nullptr;
	}
	xmprintf(9, "SHMTest::onClose() finished  \n");
}

void SHMTest::stopQt() {
	if (status != 0) return;
	using namespace boost::interprocess;
//#ifndef WIN32    // in Windows it sometimes hangs on boost::interprocess::interprocess_condition::notify_all
	onClose();
//#endif
	
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
	int result = sendCommand(CmdHeader::qSetLogLevel, level);
	//xmPrintLevel = tmp; // level
}

int SHMTest::sendCommand(CmdHeader::QWCmd cmd, const char* text, int test) {
	if (status != 0) return 0;
	using namespace boost::interprocess;
	xmprintf(4, "SHMTest::sendCommand(%d, %s): locking ..\n", static_cast<int>(cmd), text);
	scoped_lock<interprocess_mutex> lock(pd.hdr->mutex);
	xmprintf(4, "\tSHMTest::sendCommand(%d, %s): locked ..\n", static_cast<int>(cmd), text);
	pd.hdr->cmd = cmd;
	if (text != 0) {
		strncpy(pd.hdr->name, text, CmdHeader::nameSize);
	}
	pd.hdr->test = test;
#if (BOOST_VERSION >= 107800)
	boost::interprocess::cv_status wResult;		//  boost 1.78 ? 
#else
	bool wResult76 = false;							//  boost 1.76
#endif

	for (int nCounter = 16; nCounter > 0; nCounter--) {
		pd.hdr->cmdWait.notify_all();
		pd.hdr->cmdWait.notify_all();
		xmprintf(4, "\t  SHMTest::sendCommand(%d, %s): start waiting (%d) ..\n", static_cast<int>(cmd), text, nCounter);
		//pd.hdr->workDone.wait(lock);
#if (BOOST_VERSION >= 107800)
		wResult = pd.hdr->workDone.wait_for(lock, boost::chrono::milliseconds(75));
		if (wResult == cv_status::no_timeout) {  //  condition worked, no timeout 
			break;
		}
#else             // boost 1.76:
		boost::system_time const timeout = boost::get_system_time() + boost::posix_time::milliseconds(75);
		wResult76 = pd.hdr->workDone.timed_wait(lock, timeout);
		if (wResult76) { //  condition worked, no timeout 
			break;
		}
#endif
	}
#if (BOOST_VERSION >= 107800)
	if (wResult == cv_status::no_timeout) {
#else
	if (wResult76) {
#endif
		xmprintf(4, "\tSHMTest::sendCommand(%d, %s): complete\n", static_cast<int>(cmd), text);

		int test = pd.hdr->test;
		xmprintf(4, "\tSHMTest::sendCommand(%d, %s): test = %d\n", static_cast<int>(cmd), text, test);
		return test;
	} else {
		xmprintf(4, "\tSHMTest::sendCommand(%d, %s): TIMEOUT 1\n");
		return -1;
	}
	return 0;
}
int SHMTest::sendCommand(CmdHeader::QWCmd cmd, int v, unsigned int flags) {
	if (status != 0) return 0;
	using namespace boost::interprocess;
	using namespace std::chrono_literals;
	xmprintf(4, "SHMTest::sendCommand(%d, %d): locking ..\n", static_cast<int>(cmd), v);
	scoped_lock<interprocess_mutex> lock(pd.hdr->mutex);
	xmprintf(4, "\tSHMTest::sendCommand locked. \n");
	pd.hdr->cmd = cmd;
	pd.hdr->test = v;
	pd.hdr->flags = flags;

	int bv = BOOST_VERSION;
#if (BOOST_VERSION >= 107800)
	boost::interprocess::cv_status wResult;		//  boost 1.78 ? 
#else
	bool wResult76 = false;							//  boost 1.76
#endif
	for (int nCounter = 16; nCounter > 0; nCounter--) {
		pd.hdr->cmdWait.notify_all();
		pd.hdr->cmdWait.notify_all();
		xmprintf(4, "\t %d SHMTest::sendCommand(%d, %d): start waiting ..  (BOOST_VERSION =%d)\n", nCounter, static_cast<int>(cmd), v, BOOST_VERSION);
		////pd.hdr->workDone.wait(lock);
		  //    boost 1.78
#if (BOOST_VERSION >= 107800)
		wResult = pd.hdr->workDone.wait_for(lock, 78ms);
		if (wResult == cv_status::no_timeout) {  //  condition worked, no timeout 
			xmprintf(8, "\t\t (%d) (%d, %d):  condition worked, no timeout \n", nCounter, static_cast<int>(cmd), v);
			break;
		} else {
			xmprintf(8, "\t\t (%d) (%d, %d): timeout. trying one more time.. \n", nCounter, static_cast<int>(cmd), v);
		}
#else             // boost 1.76:
		boost::system_time const timeout = boost::get_system_time() + boost::posix_time::milliseconds(75);
		wResult76 = pd.hdr->workDone.timed_wait(lock, timeout);
		if (wResult76) { //  condition worked, no timeout 
			break;
		}
#endif
	}
#if (BOOST_VERSION >= 107800)
	if (wResult == cv_status::no_timeout) {
#else
	if (wResult76) {
#endif
		xmprintf(4, "\tSHMTest::sendCommand(%d, %d): finished\n", static_cast<int>(cmd), v);
		int test = pd.hdr->test;
		xmprintf(4, "\tSHMTest::sendCommand(%d, %d): test = %d\n", static_cast<int>(cmd), v, test);
		return test;
	} else {
		xmprintf(4, "\tSHMTest::sendCommand(%d, %s) 2: TIMEOUT\n");
		return -1;
	}
	return 0;
}

void SHMTest::cbThreadF() {
	using namespace boost::interprocess;
	CBPickerInfo cpi;
	//cv_status status;   //    will be OK since boost 1.78 probably

	//   fix for old BOOST
	bool noTimeout = true;
	boost::system_time timeout;

	//scoped_lock<interprocess_mutex> lock(pd.hdr->cbInfoMutex);
	scoped_lock<named_mutex> lock(cmdSync->cbInfoMutex);
	while (!needStopCallbackThread) {
		//cmdSync->cbWait.wait(lock); //   wait for the picker info

		  //    will be OK since boost 1.78 probably
		//if ((status = cmdSync->cbWait.wait_for(lock, boost::chrono::milliseconds(200))) == cv_status::timeout) { //   wait for the picker info

		//  fix for old BOOST
		timeout=boost::get_system_time() + boost::posix_time::milliseconds(200);
		if (noTimeout = cmdSync->cbWait.timed_wait(lock, timeout)) { //   wait for the picker info
			if (needStopCallbackThread) {
				cbiReady.notify_all();
				break;
			}
			continue;
		}
		if (needStopCallbackThread) {
			cbiReady.notify_all();
			break;
		}

		memcpy(&cpi, &pd.hdr->cbInfo, sizeof(cpi));
		//xmprintf(5, "cbThreadF   time = %f \n", cpi.time);

		cbiMutex_3.lock();
		memcpy(&cpInfo, &cpi, sizeof(cpi));
		cbiMutex_3.unlock();

		cbiReady.notify_all();
	}
}

void SHMTest::cbThreadF_2() {
	std::unique_lock<std::mutex> lock(cbiMutex_2);
	CBPickerInfo cpi;
	std::cv_status status; 
	while (!needStopCallbackThread) {
		if ((status = cbiReady.wait_for(lock, std::chrono::milliseconds(200))) == std::cv_status::timeout) {
			if (needStopCallbackThread) {
				break;
			}
			continue;
		}
		if (needStopCallbackThread) {
			break;
		}

		if (pCallback != 0) {
			cbiMutex_3.lock();
			memcpy(&cpi, &cpInfo, sizeof(cpi));
			cbiMutex_3.unlock();

			// int figureID, int lineID, int index, int fx, int fy, double x, double y, double z, double t, const std::string& legend
			//pCallback(cpInfo.plotID, cpInfo.lineID, cpInfo.index, cpInfo.xx, cpInfo.yy, cpInfo.x, cpInfo.y, cpInfo.z, cpInfo.time, cpInfo.label);

			xmprintf(4, "cbThreadF_2! time = %.3f \n", cpInfo.time);
		}
		if (cbTest1 != 0) {
			try {
				cbTest1();
			}	catch (const std::exception& ex) {
				xmprintf(4, "cbThreadF_2: exception {%s} \n", ex.what());
			}
		}

		if (cbTest2 != 0) {
			cbiMutex_3.lock();
			memcpy(&cpi, &cpInfo, sizeof(cpi));
			cbiMutex_3.unlock();

			cbTest2(cpi.index);
		}
	}
}

void SHMTest::setCB(OnPCallback cb) {
	pCallback = cb;
}
void SHMTest::setClipCallback(OnClipCallback cb) {
	pClipCallback = cb;
}
void SHMTest::setCBTest1(CBTest_1 cb) {
	cbTest1 = cb;
}

void SHMTest::setCBTest2(CBTest_2 cb) {
	cbTest2 = cb;
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

int SHMTest::qwtsave_png(int id, char* filename) {
	if (status != 0) return -7;
	int result = sendCommand(CmdHeader::qSavePng, filename, id);
	xmprintf(4, "\tSHMTest::qwtsave_png(id = %d); result = %d\n", id, result);
	return result;
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
