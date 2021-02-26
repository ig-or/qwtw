
#include "xstdef.h"
#include "qwproto.h"
#include "qwproc.h"
#include "qworker.h"
#include <QApplication>

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

QProcInterface::QProcInterface(QWorker& worker_, QApplication& app_): 
		worker(worker_), app(app_) {
	shmCommand = 0;
	shmDataX = 0;
	shmDataY = 0;
	shmDataZ = 0;
	shmDataT = 0;
	shmDataData = 0;
	started = false;
}

QProcInterface::~QProcInterface() {
	

	stop();

	//    looks like we do not need it here:
	//if (wThread.joinable()) {
	//	wThread.join();
	//}

	//if (shmCommand != 0) {
		removeSHM();
	//}
}

void QProcInterface::removeSHM() {
	using namespace boost::interprocess;
	shared_memory_object::remove(ProcData::shmNames[0]);
	shared_memory_object::remove(ProcData::shmNames[1]);
	shared_memory_object::remove(ProcData::shmNames[2]);
	shared_memory_object::remove(ProcData::shmNames[3]);
	shared_memory_object::remove(ProcData::shmNames[4]);
	shared_memory_object::remove(ProcData::shmNames[5]);
}

/*
void QProcInterface::setupSHM1(long long size, 
		boost::interprocess::shared_memory_object* shm, 
		boost::interprocess::mapped_region* reg) {
	
}
*/
bool QProcInterface::runningAlready() {
	using namespace boost::interprocess;
	bool ret = false;
	try {
		shared_memory_object shmCommandTest(open_only, ProcData::shmNames[0], read_write);
		ret = true;
	} catch(interprocess_exception &ex) { 

	}
	return ret;
}


void QProcInterface::start() {
	xmprintf(3, "QProcInterface::start() starting\n");
	if (started) {
		xmprintf(3, "\t - already\n");
		return;
	}
	const long long startSegSize = 1024;
	using namespace boost::interprocess;
	//Create a shared memory object.
	removeSHM();
	xmprintf(3, "/tQProcInterface::start() setting up SHM\n");
	try{

		shmCommand =  new shared_memory_object(create_only, ProcData::shmNames[0], read_write);
		shmDataX =  new shared_memory_object(create_only, ProcData::shmNames[1], read_write);
		shmDataY =  new shared_memory_object(create_only, ProcData::shmNames[2], read_write);
		shmDataZ =  new shared_memory_object(create_only, ProcData::shmNames[3], read_write);
		shmDataT =  new shared_memory_object(create_only, ProcData::shmNames[4], read_write);

		shmDataData =  new shared_memory_object(create_only, ProcData::shmNames[5], read_write);
	} catch (interprocess_exception &ex){
		xmprintf(0, "QProcInterface::start()  cannot create shared memory: %s \n", ex.what());
		return;
	}
	shmCommand->truncate(sizeof(CmdHeader));
	shmDataX->truncate(sizeof(double) * startSegSize);
	shmDataY->truncate(sizeof(double) * startSegSize);
	shmDataZ->truncate(sizeof(double) * startSegSize);
	shmDataT->truncate(sizeof(double) * startSegSize);
	shmDataData->truncate(sizeof(double) * startSegSize);

	commandReg = new mapped_region(*shmCommand, read_write);
	xDataReg   = new mapped_region(*shmDataX, read_write);
	yDataReg   = new mapped_region(*shmDataY, read_write);
	zDataReg   = new mapped_region(*shmDataZ, read_write);
	tDataReg   = new mapped_region(*shmDataT, read_write);
	dataDataReg   = new mapped_region(*shmDataData, read_write);

	//pd.hdr = (CmdHeader*)commandReg->get_address();
	pd.hdr = new (commandReg->get_address()) CmdHeader;
	xmprintf(3, "/tQProcInterface::start() locking..\n");
	scoped_lock<interprocess_mutex> lock(pd.hdr->mutex);
	xmprintf(3, "/tQProcInterface::start() locked.\n");
	pd.hdr->cmd = 100; //    just started
	pd.hdr->segSize = startSegSize;
	pd.hdr->dataSize = startSegSize;

	pd.x = static_cast<double*>(xDataReg->get_address());
	pd.y = static_cast<double*>(yDataReg->get_address());
	pd.z = static_cast<double*>(zDataReg->get_address());
	pd.t = static_cast<double*>(tDataReg->get_address());
	pd.data = static_cast<double*>(dataDataReg->get_address());

	needStopThread = false;

	//std::thread ttmp(&QProcInterface::run, this);
	//ttmp.swap(wThread);
	xmprintf(3, "\tQProcInterface::start() starting interface thread.. \n");
	wThread = std::make_shared<boost::thread>(&QProcInterface::run, this);
	
	started = true;
	xmprintf(3, "\tQProcInterface::start()  finished\n");
}

void QProcInterface::stop() {
	using namespace boost::interprocess;
	xmprintf(3, "QProcInterface::stop()  \n");
	if (wThread->joinable()) {
		needStopThread = true;
		xmprintf(3, "\t QProcInterface::stop()  locking..\n");
		pd.hdr->mutex.lock();
		xmprintf(3, "\t QProcInterface::stop()  locked\n");
		pd.hdr->cmd = CmdHeader::exit;
		pd.hdr->mutex.unlock();
		xmprintf(3, "\t QProcInterface::stop()  unlocked\n");
		pd.hdr->cmdWait.notify_all();
	
		wThread->join();
		xmprintf(3, "\t QProcInterface::stop()  join finished\n");
	} else {
		xmprintf(3, "\tQProcInterface::stop()  thread not joinable \n");
	}
	xmprintf(3, "\tQProcInterface::stop() finished \n");
}

void QProcInterface::run() {
	using namespace boost::interprocess;
	xmprintf(2, "QProcInterface::run() starting\n");
	xmprintf(5, "\tQProcInterface::run() locking..\n");
	scoped_lock<interprocess_mutex> lock(pd.hdr->mutex);
	xmprintf(6, "\tQProcInterface::run() locked\n");
	while (!needStopThread) {
		//   wait for another command
		xmprintf(5, "\tQProcInterface::run() waiting.. \n");
		pd.hdr->cmdWait.wait(lock);
		xmprintf(3, "\tQProcInterface::run()   after pd.hdr->cmdWait.wait(lock);  \n ");
		int cmd = pd.hdr->cmd;
		processCommand(cmd);
		xmprintf(6, "\tQProcInterface::run() before notify_all\n");
		pd.hdr->workDone.notify_all();
		xmprintf(6, "\tQProcInterface::run() after notify_all\n");
	}
	xmprintf(2, "QProcInterface::run() exiting \n");
}

void QProcInterface::changeDataSize(long long newSize) {
	using namespace boost::interprocess;
	if (newSize <= pd.hdr->dataSize) {
		return;
	}
	newSize <<= 1;
	long long s1 = newSize * sizeof(double);
	shmDataData->truncate(s1);
	delete dataDataReg;
	dataDataReg = new mapped_region(*shmDataData, read_write); 
	pd.data =  static_cast<double*>(dataDataReg->get_address());
	pd.hdr->dataSize = newSize;
}

void QProcInterface::changeSize(long long newSize) {
	using namespace boost::interprocess;
	if (newSize <= pd.hdr->segSize) {
		return;
	}
	newSize <<= 1;
	long long s1 = newSize * sizeof(double);
	shmDataX->truncate(s1);
	shmDataY->truncate(s1);
	shmDataZ->truncate(s1);
	shmDataT->truncate(s1);
	delete xDataReg;
	delete yDataReg;
	delete zDataReg;
	delete tDataReg;
	xDataReg   = new mapped_region(*shmDataX, read_write);
	yDataReg   = new mapped_region(*shmDataY, read_write);
	zDataReg   = new mapped_region(*shmDataZ, read_write);
	tDataReg   = new mapped_region(*shmDataT, read_write);

	pd.x = static_cast<double*>(xDataReg->get_address());
	pd.y = static_cast<double*>(yDataReg->get_address());
	pd.z = static_cast<double*>(zDataReg->get_address());
	pd.t = static_cast<double*>(tDataReg->get_address());

	pd.hdr->segSize = newSize;
}

void QProcInterface::plot() {
	worker.qwtplot(pd.x, pd.y, pd.hdr->size, pd.hdr->name, pd.hdr->style, pd.hdr->lineWidth, pd.hdr->symSize);
}

void QProcInterface::processCommand(int cmd) {
	xmprintf(2, "QProcInterface::processCommand got cmd = %d \n", cmd);

	switch(cmd) {
		case CmdHeader::exit:
			xmprintf(2, "QProcInterface::processCommand : sending QUIT to QT..  \n");
			QMetaObject::invokeMethod(&app, "quit", Qt::BlockingQueuedConnection); // QueuedConnection
			xmprintf(2, "QProcInterface::processCommand : QUIT was sent \n");
			needStopThread = true;
			//now  lets wait for the app to exit
			{
				//xm_printf("QProcInterface::processCommand :  lets wait for the app to exit..  \n");
				//std::unique_lock<std::mutex> lck(worker.appMutex);
				//worker.appV.wait(lck);
				//xm_printf("QProcInterface::processCommand : QT app looks like exited! \n");
			}
			
			break;
		case CmdHeader::changeSize:
			changeSize(pd.hdr->size);
			break;

		case CmdHeader::changeDataSize:
			changeDataSize(pd.hdr->size);
			break;

		case CmdHeader::qMW:
			worker.qwtshowmw();
			break;

		case CmdHeader::qFigure:
			worker.qwtfigure(pd.hdr->test);
			break;

		case CmdHeader::qRemoveLine:
			worker.qwtRemoveLine(pd.hdr->test);
			break;

		case CmdHeader::qMap:
			worker.mapview(pd.hdr->test);
			break;

#ifdef USEMATHGL
		case CmdHeader::qMglPlot:
			worker.mglPlot(pd.hdr->test);
			break;
		case CmdHeader::qMglLine:
			xmprintf(5, "\tcase CmdHeader::qMglLine\n");
			if (pd.hdr->size <= pd.hdr->segSize) {
				xmprintf(6, "\tpd.hdr->size=%d;   pd.hdr->segSize=%d   \n", pd.hdr->size, pd.hdr->segSize);
				worker.mgl_line(pd.hdr->size,  pd.x, pd.y, pd.z, pd.hdr->name, pd.hdr->style);
			} else {
				xmprintf(0, "\tERROR CmdHeader::qMglLine pd.hdr->size=%d;   pd.hdr->segSize=%d   \n", pd.hdr->size, pd.hdr->segSize);
			}
			break;
		case CmdHeader::qMglMesh:
			if ((pd.hdr->xSize * pd.hdr->ySize) <= pd.hdr->dataSize) {
				worker.mgl_mesh(pd.hdr->xSize, pd.hdr->ySize, 
					pd.hdr->xMin, pd.hdr->xMax, pd.hdr->yMin, pd.hdr->yMax,
					pd.data, pd.hdr->name, pd.hdr->style, pd.hdr->type);
			} else {
				xmprintf(0, "CmdHeader::qMglMesh: data size error; xSize = %d; ySize = %d; dataSize = %d\n",
					pd.hdr->xSize, pd.hdr->ySize, pd.hdr->dataSize);
			}
			break;
#endif

		case CmdHeader::qTitle:
			worker.qwttitle(pd.hdr->name);
			break;
			
		case CmdHeader::qXlabel:
			worker.qwtxlabel(pd.hdr->name);
			break;
			
		case CmdHeader::qYlabel:
			worker.qwtylabel(pd.hdr->name);
			break;
			
		case CmdHeader::qClear:
			worker.qwtclear();
			break;
			
		case CmdHeader::qImpStatus:
			worker.qwtsetimpstatus(pd.hdr->test);
			break;

		case CmdHeader::qPlot:
			if (pd.hdr->size <= pd.hdr->segSize) {
				xmprintf(5, "processCommand qPlot; style = [%s]\n", pd.hdr->style);
				int test = worker.qwtplot(pd.x, pd.y, pd.hdr->size, pd.hdr->name, pd.hdr->style, pd.hdr->lineWidth, pd.hdr->symSize);
				pd.hdr->test = test; //   put answer here
				xmprintf(5, "processCommand qPlot; worker complete;\n");
			}
			break;

		case CmdHeader::qPlot2:
			if (pd.hdr->size <= pd.hdr->segSize) {
				xmprintf(5, "processCommand qPlot2; style = [%s]\n", pd.hdr->style);
				int test = worker.qwtplot2(pd.x, pd.y, pd.hdr->size, pd.hdr->name, pd.hdr->style,
					pd.hdr->lineWidth, pd.hdr->symSize, pd.t);
				pd.hdr->test = test; //   put answer here
				xmprintf(5, "processCommand qPlot2; worker complete;\n");
			}
			break;

		case CmdHeader::qChangeLine:
			if (pd.hdr->size <= pd.hdr->segSize) {
				xmprintf(5, "processCommand qChangeLine; size = [%d]\n", pd.hdr->size);
				double* t = (pd.hdr->timeIsGood == 0) ? 0 : pd.t;
				double* z = (pd.hdr->zIsGood == 0) ? 0 : pd.z;
				int test = worker.qwtChangeLine(pd.hdr->test,  pd.x, pd.y, z, t, pd.hdr->size);
				pd.hdr->test = test; //   put answer here
				xmprintf(5, "processCommand qChangeLine; worker complete;\n");
			}
			break;

		case CmdHeader::qEnableBC:
			worker.qwtEnableCoordBroadcast(pd.x, pd.y, pd.z, pd.t, pd.hdr->size);
			break;

		case CmdHeader::qDisableBC:
			worker.qwtDisableCoordBroadcast();
			break;

		case CmdHeader::qSetLogLevel:
			//xmPrintLevel = pd.hdr->test;
			xmprintf(1, "got qSetLogLevel with [%d]\n", pd.hdr->test);
			break;
	};
	xmprintf(2, "QProcInterface::processCommand  cmd = %d finished\n", cmd);

}


