
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
	if (started) {
		return;
	}
	const long long startSegSize = 1024;
	using namespace boost::interprocess;
	//Create a shared memory object.
	removeSHM();
	try{

		shmCommand =  new shared_memory_object(create_only, ProcData::shmNames[0], read_write);
		shmDataX =  new shared_memory_object(create_only, ProcData::shmNames[1], read_write);
		shmDataY =  new shared_memory_object(create_only, ProcData::shmNames[2], read_write);
		shmDataZ =  new shared_memory_object(create_only, ProcData::shmNames[3], read_write);
		shmDataT =  new shared_memory_object(create_only, ProcData::shmNames[4], read_write);
	} catch (interprocess_exception &ex){
		xm_printf("cannot create shared memory: %s \n", ex.what());
		return;
	}
	shmCommand->truncate(sizeof(CmdHeader));
	shmDataX->truncate(sizeof(double) * startSegSize);
	shmDataY->truncate(sizeof(double) * startSegSize);
	shmDataZ->truncate(sizeof(double) * startSegSize);
	shmDataT->truncate(sizeof(double) * startSegSize);

	commandReg = new mapped_region(*shmCommand, read_write);
	xDataReg   = new mapped_region(*shmDataX, read_write);
	yDataReg   = new mapped_region(*shmDataY, read_write);
	zDataReg   = new mapped_region(*shmDataZ, read_write);
	tDataReg   = new mapped_region(*shmDataT, read_write);

	//pd.hdr = (CmdHeader*)commandReg->get_address();
	pd.hdr = new (commandReg->get_address()) CmdHeader;
	scoped_lock<interprocess_mutex> lock(pd.hdr->mutex);
	pd.hdr->cmd = 100; //    just started
	pd.hdr->segSize = startSegSize;

	pd.x = static_cast<double*>(xDataReg->get_address());
	pd.y = static_cast<double*>(yDataReg->get_address());
	pd.z = static_cast<double*>(zDataReg->get_address());
	pd.t = static_cast<double*>(tDataReg->get_address());

	needStopThread = false;
	std::thread ttmp(&QProcInterface::run, this);
	ttmp.swap(wThread);

	started = true;
}

void QProcInterface::stop() {
	using namespace boost::interprocess;
	if (wThread.joinable()) {
		needStopThread = true;
		pd.hdr->mutex.lock();
		pd.hdr->cmd = CmdHeader::exit;
		pd.hdr->mutex.unlock();
		pd.hdr->cmdWait.notify_all();
	
		wThread.join();
	}
}

void QProcInterface::run() {
	using namespace boost::interprocess;
	//xm_printf("QProcInterface::run() 1\n");
	while (!needStopThread) {
		//   wait for another command
		scoped_lock<interprocess_mutex> lock(pd.hdr->mutex);
		pd.hdr->cmdWait.wait(lock);
		//xm_printf("QProcInterface::run()   after pd.hdr->cmdWait.wait(lock);  \n ");
		int cmd = pd.hdr->cmd;
		processCommand(cmd);
		pd.hdr->workDone.notify_all();
	}
	//xm_printf("QProcInterface::run() exiting \n");
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
	xm_printf("QProcInterface::processCommand got cmd = %d \n", cmd);

	switch(cmd) {
		case CmdHeader::exit:
			//xm_printf("QProcInterface::processCommand : sending QUIT to QT..  \n");
			QMetaObject::invokeMethod(&app, "quit", Qt::BlockingQueuedConnection); // QueuedConnection
			//xm_printf("QProcInterface::processCommand : QUIT was sent \n");
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

		case CmdHeader::qMW:
			worker.qwtshowmw();
			break;

		case CmdHeader::qFigure:
			worker.qwtfigure(pd.hdr->test);
			break;

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
				//xm_printf("qPlot: style = [%s]\n", pd.hdr->style);
				worker.qwtplot(pd.x, pd.y, pd.hdr->size, pd.hdr->name, pd.hdr->style, pd.hdr->lineWidth, pd.hdr->symSize);
			}
			break;

		case CmdHeader::qPlot2:
			if (pd.hdr->size <= pd.hdr->segSize) {
				//xm_printf("qPlot: style = [%s]\n", pd.hdr->style);
				worker.qwtplot2(pd.x, pd.y, pd.hdr->size, pd.hdr->name, pd.hdr->style, 
					pd.hdr->lineWidth, pd.hdr->symSize, pd.t);
			}
			break;

		case CmdHeader::qEnableBC:
			worker.qwtEnableCoordBroadcast(pd.x, pd.y, pd.z, pd.t, pd.hdr->size);
			break;

		case CmdHeader::qDisableBC:
			worker.qwtDisableCoordBroadcast();
			break;


	};

}

