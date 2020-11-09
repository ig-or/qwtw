

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
	shmDataT = 0;
	started = false;
}

QProcInterface::~QProcInterface() {
	using namespace boost::interprocess;

	stop();

	//    looks like we do not need it here:
	//if (wThread.joinable()) {
	//	wThread.join();
	//}

	//if (shmCommand != 0) {
		shared_memory_object::remove("QWTWCommand");
	//}
}


void QProcInterface::start() {
	if (started) {
		return;
	}
	using namespace boost::interprocess;
	//Create a shared memory object.
	shared_memory_object::remove("QWTWCommand");
	try{

		shmCommand =  new shared_memory_object(create_only, "QWTWCommand", read_write);
	} catch (interprocess_exception &ex){
		printf("cannot create shared memory: %s \n", ex.what());
		return;
	}
	shmCommand->truncate(sizeof(CmdHeader));
	commandReg = new mapped_region(*shmCommand, read_write);

	//pd.hdr = (CmdHeader*)commandReg->get_address();
	pd.hdr = new (commandReg->get_address()) CmdHeader;
	scoped_lock<interprocess_mutex> lock(pd.hdr->mutex);
	pd.hdr->cmd = 100; //    just started

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
	printf("QProcInterface::run() 1\n");
	while (!needStopThread) {
		//   wait for another command
		scoped_lock<interprocess_mutex> lock(pd.hdr->mutex);
		pd.hdr->cmdWait.wait(lock);
		printf("QProcInterface::run()   after pd.hdr->cmdWait.wait(lock);  \n ");
		int cmd = pd.hdr->cmd;
		processCommand(cmd);
		pd.hdr->workDone.notify_all();
	}
	printf("QProcInterface::run() exiting \n");
}

void QProcInterface::processCommand(int cmd) {
	printf("QProcInterface::processCommand got cmd = %d \n", cmd);

	switch(cmd) {
		case CmdHeader::exit:
			printf("QProcInterface::processCommand : sending QUIT to QT..  \n");
			QMetaObject::invokeMethod(&app, "quit", Qt::BlockingQueuedConnection); // QueuedConnection
			printf("QProcInterface::processCommand : QUIT was sent \n");
			needStopThread = true;
			//now  lets wait for the app to exit
			{
				//printf("QProcInterface::processCommand :  lets wait for the app to exit..  \n");
				//std::unique_lock<std::mutex> lck(worker.appMutex);
				//worker.appV.wait(lck);
				//printf("QProcInterface::processCommand : QT app looks like exited! \n");
			}
			
			break;

	};

}

