

#include "qwproto.h"
#include "qwproc.h"

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

QProcInterface::QProcInterface() {
	shmCommand = 0;
	shmDataX = 0;
	shmDataY = 0;
	shmDataT = 0;
	started = false;
}

QProcInterface::~QProcInterface() {
	using namespace boost::interprocess;

	stop();
	
	if (shmCommand != 0) {
		shared_memory_object::remove("QWTWCommand");
	}

}


void QProcInterface::start() {
	if (started) {
		return;
	}
	using namespace boost::interprocess;
	//Create a shared memory object.
	shmCommand =  new shared_memory_object(create_only, "QWTWCommand", read_write);
	shmCommand->truncate(sizeof(CmdHeader));
	commandReg = new mapped_region(*shmCommand, read_write);

	pd.hdr = (CmdHeader*)commandReg->get_address();

	std::thread ttmp(&QProcInterface::run, this);
	ttmp.swap(wThread);

	started = true;
}

void QProcInterface::stop() {

	if (wThread.joinable()) {
		wThread.join();
	}
}

void QProcInterface::run() {
	printf("QProcInterface::run()\n");
}

