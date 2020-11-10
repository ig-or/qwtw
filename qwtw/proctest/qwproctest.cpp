#include "qwproc.h"

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <thread>
#include <chrono>
#include <iostream>

#define xm_printf printf

ProcData pd;

struct SHMTest {

	SHMTest() {

	}
	void testInit() {
		using namespace boost::interprocess;

		shared_memory_object shmCommand_(open_only, ProcData::shmNames[0], read_write);
		shared_memory_object shmX_(open_only, ProcData::shmNames[1], read_write);
		shared_memory_object shmY_(open_only, ProcData::shmNames[2], read_write);
		shared_memory_object shmZ_(open_only, ProcData::shmNames[3], read_write);

		shmCommand.swap(shmCommand_);
		shmX.swap(shmX_);
		shmY.swap(shmY_);
		shmZ.swap(shmZ_);

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
		}

		mapped_region xReg_ = mapped_region(shmX, read_write);
		mapped_region yReg_ = mapped_region(shmY, read_write);
		mapped_region tReg_ = mapped_region(shmZ, read_write);
		xReg.swap(xReg_);
		yReg.swap(yReg_);
		tReg.swap(tReg_);

		pd.x = static_cast<double*>(xReg.get_address());
		pd.y = static_cast<double*>(yReg.get_address());
		pd.t = static_cast<double*>(tReg.get_address());
	}

	void stopQt() {
		using namespace boost::interprocess;
		
		scoped_lock<interprocess_mutex> lock(pd.hdr->mutex);
		printf("cmd = %d  \n", pd.hdr->cmd);
		pd.hdr->cmd = CmdHeader::exit;
		pd.hdr->cmdWait.notify_all();
		printf("TEST: start waiting ..\n");
		pd.hdr->workDone.wait(lock);
	}

	void drawMW() {
		using namespace boost::interprocess;
		scoped_lock<interprocess_mutex> lock(pd.hdr->mutex);
		pd.hdr->cmd = CmdHeader::qMW;
		pd.hdr->cmdWait.notify_all();
		printf("TEST: start waiting ..\n");
		pd.hdr->workDone.wait(lock);
	}



	void changeSize(long long newSize) {
		using namespace boost::interprocess;
		scoped_lock<interprocess_mutex> lock(pd.hdr->mutex);
		pd.hdr->cmd = CmdHeader::changeSize;
		pd.hdr->size = newSize;

		pd.hdr->cmdWait.notify_all();
		printf("TEST: start waiting ..\n");
		pd.hdr->workDone.wait(lock);
	}

	void plotSomethingFinally() {
		using namespace boost::interprocess;
		scoped_lock<interprocess_mutex> lock(pd.hdr->mutex);
		pd.hdr->cmd = CmdHeader::qPlot;
		pd.hdr->lineWidth = 3;
		pd.hdr->symSize = 5;
		strncpy(pd.hdr->style, "-eb", 8);
		double x[] = {0., 1., 2., 3., 4.};
		double y[] = {1., 2.4, 2.8, 1.8, 0.1};

		memcpy(pd.x, x, sizeof(double) * 5);
		memcpy(pd.y, y, sizeof(double) * 5);
		pd.hdr->size = 5;
		strncpy(pd.hdr->name, "test name", pd.hdr->nameSize);

		pd.hdr->cmdWait.notify_all();
		printf("TEST: start waiting ..\n");
		pd.hdr->workDone.wait(lock);

	}

	void plotSomethingBig() {
		using namespace boost::interprocess;
		printf("TEST: plotSomethingBig start \n");
		scoped_lock<interprocess_mutex> lock(pd.hdr->mutex);

		//  prepare the data
		const long long bsize = 50000;
		double x[bsize];
		double y[bsize];
		for (int i = 0; i < bsize; i++) {
			x[i] = i * 0.0045;
			y[i] = sin(x[i]) + cos(x[i] * 1.4);
		}
		
		//   check max size on the other side:
		long long a = pd.hdr->segSize;
		if (a < bsize) {
			printf("TEST: inc seg size (1); current size = %lld \n", a);
			pd.hdr->cmd = CmdHeader::changeSize;
			pd.hdr->size = bsize;	
			pd.hdr->cmdWait.notify_all();
			pd.hdr->workDone.wait(lock);

			//  now we have to adjust our memory somehow..
			long long segSize = pd.hdr->segSize;
			printf("TEST: new size is %lld \n", segSize);
					
			shmX.truncate(segSize * sizeof(double));
			shmY.truncate(segSize * sizeof(double));
			shmZ.truncate(segSize * sizeof(double));

			xReg = mapped_region(shmX, read_write);
			yReg = mapped_region(shmY, read_write);
			tReg = mapped_region(shmZ, read_write);

			pd.x = static_cast<double*>(xReg.get_address());
			pd.y = static_cast<double*>(yReg.get_address());
			pd.t = static_cast<double*>(tReg.get_address());
		}

		//  now lets plot
		pd.hdr->cmd = CmdHeader::qPlot;
		pd.hdr->lineWidth = 3;
		pd.hdr->symSize = 5;
		strncpy(pd.hdr->style, "-m", 8);
		pd.hdr->size = bsize;
		strncpy(pd.hdr->name, "big test name", pd.hdr->nameSize);

		memcpy(pd.x, x, sizeof(double) * bsize);
		memcpy(pd.y, y, sizeof(double) * bsize);
		
		
		pd.hdr->cmdWait.notify_all();
		printf("TEST: start waiting plotSomethingBig..\n");
		pd.hdr->workDone.wait(lock);
		printf("TEST: plotSomethingBig.. done\n");

	}

	boost::interprocess::shared_memory_object shmCommand;
	boost::interprocess::shared_memory_object shmX;
	boost::interprocess::shared_memory_object shmY;
	boost::interprocess::shared_memory_object shmZ;

	boost::interprocess::mapped_region commandReg;
	boost::interprocess::mapped_region xReg;
	boost::interprocess::mapped_region yReg;
	boost::interprocess::mapped_region tReg;
};



int main(int argc, char** argv) {
	setbuf(stdout, NULL);
	setvbuf(stdout, NULL, _IONBF, 0);
   	using namespace boost::interprocess;
	using namespace std::chrono_literals;

	printf("TEST main: starting \n");
	SHMTest test;
	test.testInit();
	test.drawMW();
	std::this_thread::sleep_for(2s);
	test.plotSomethingFinally();
	std::this_thread::sleep_for(2s);
	test.plotSomethingBig();
	//std::this_thread::sleep_for(8s);
	int tmp;
	std::cin >> tmp;
	test.stopQt();
	printf("TEST main: exiting \n");
	return 0;
}


void assert_failed(const char* file, unsigned int line, const char* str) {
	xm_printf("ASSERT faild: %s (file %s, line %d)\n", str, file, line);
}


