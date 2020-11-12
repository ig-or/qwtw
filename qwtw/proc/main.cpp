



#include "qwproc.h"
#include "xstdef.h"
#include "qworker.h"
#include "qwproto.h"

#include <QObject>
#include <QPointer>
#include <QApplication>

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <stdlib.h>

#include <string>
#include <iostream>
#include <sstream>

#include "boost/filesystem.hpp"


static QPointer<QWorker> q2worker(nullptr);

int lockHandle() {
	using namespace boost::filesystem;
	boost::system::error_code ec;
	path home(getenv("HOME"));
	path dir = home / ".qwtw" / "lock";
	path f = dir / "qwproc";

	// install our lock
	pid_t pid = getpid(); //  my PID
	if (!exists(dir, ec)) {
		bool lockDirCreated = create_directories(dir, ec);
		if (!lockDirCreated) {
			printf("cannot create dirs; error %s\n", ec.message().c_str());
			return 2;
		}
	}
	std::ofstream lockFile(f.string());
	lockFile << pid;
	lockFile.close();
	return 0;
}

int main(int argc, char** argv) {
	bool test = QProcInterface::runningAlready();
	if (test) {
		printf("shm exists\n");
		//return 2;
	}

	//  code partially from https://github.com/pasce/daemon-skeleton-linux-c:
	pid_t pid;
	/* Fork off the parent process */
	pid = fork();
	/* An error occurred */
	if (pid < 0) {
		exit(EXIT_FAILURE);
	}
	
	 /* Success: Let the parent terminate */
	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}
	
	/* On success: The child process becomes session leader */
	if (setsid() < 0) {
		exit(EXIT_FAILURE);
	}
	
	/* Catch, ignore and handle signals */
	/*TODO: Implement a working signal handler */
	//signal(SIGCHLD, SIG_IGN);
	//signal(SIGHUP, SIG_IGN);
	
	/* Fork off for the second time*/
	pid = fork();
	
	/* An error occurred */
	if (pid < 0) {
		exit(EXIT_FAILURE);
	}
	
	/* Success: Let the parent terminate */
	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}
	
	/* Set new file permissions */
	umask(0);
	
	/* Change the working directory to the root directory */
	/* or another appropriated directory */
	//chdir("/");

	int lockState = checkProcRunning();
	if (lockState != 0) {
		printf("already started\n\n");
		exit(EXIT_FAILURE);
	}
	if (lockHandle() != 0) {
		printf("cannot setup a lock\n\n");
		exit(EXIT_FAILURE);
	}

	/* Close all open file descriptors */
	int x3;
	for (x3 = sysconf(_SC_OPEN_MAX); x3>=0; x3--)
	{
		close (x3);
	}

	/* Open the log file */
	//openlog ("firstdaemon", LOG_PID, LOG_DAEMON);

	//setbuf(stdout, NULL);
	//setvbuf(stdout, NULL, _IONBF, 0);
	//printf("main: starting \n");

	QApplication app(argc, argv);
	app.setQuitOnLastWindowClosed(false);   
	QWorker qWorker;
	qWorker.qtstart(false);
	QProcInterface qpi(qWorker, app);
	qpi.start();
	QObject::connect(&app, SIGNAL(aboutToQuit()), &qWorker, SLOT(onQtAppClosing()));
	//qWorker.appMutex.lock();
	//printf("main: starting QT thread  \n");
	app.exec();
	//printf("main: QT thread finished  \n");
	//qWorker.appMutex.unlock();
	//qWorker.appV.notify_all();
	
	//printf("main: exiting \n");
	return 0;
}


void assert_failed(const char* file, unsigned int line, const char* str) {
	xm_printf("ASSERT faild: %s (file %s, line %d)\n", str, file, line);
}


