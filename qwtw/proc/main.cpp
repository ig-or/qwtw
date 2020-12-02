



#include "qwproc.h"
#include "xstdef.h"
#include "qworker.h"
#include "qwproto.h"

#include <QObject>
#include <QPointer>
#include <QApplication>
#include <QIcon>
#include <QMainWindow>

#include <stdlib.h>

#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef WIN32
	#include "windows.h"
	#include <Shlobj.h>
#else // linux?
	#include <syslog.h>
	#include <unistd.h>
#endif

#include <string>
#include <iostream>
#include <sstream>

#include "boost/filesystem.hpp"


static QPointer<QWorker> q2worker(nullptr);
static FILE* logFile = 0;

int lockHandle() {
	using namespace boost::filesystem;
	boost::system::error_code ec;
	char p[512];
	if (getFolderLocation(p, 512)) {

	} else {
		xm_printf("ERROR: getFolderLocation not working \n");
		return 3;
	}

	path home(p);
	path dir = home / "lock";
	path f = dir / "qwproc";

	if (!exists(dir, ec)) {
		bool lockDirCreated = create_directories(dir, ec);
		if (!lockDirCreated) {
			xm_printf("cannot create dirs; error %s\n", ec.message().c_str());
			return 2;
		}
	}

	// install our lock
#ifdef WIN32
	DWORD pid = GetCurrentProcessId();
#else
	pid_t pid = getpid(); //  my PID
#endif

	std::ofstream lockFile(f.string());
	lockFile << pid;
	lockFile.close();
	xm_printf("lockHandle() complete\n");
	return 0;
}

int main(int argc, char** argv) {
	bool test = QProcInterface::runningAlready();
	if (test) {
		//xm_printf("shm exists\n");
		//return 2;
	}

#ifdef WIN32	// make it a windows service could be a better way, though.. 
				//  probably this also works:

				
#else
	//  code partially from https://github.com/pasce/daemon-skeleton-linux-c:
	pid_t pid;
	// Fork off the parent process 
	pid = fork();
	// An error occurred 
	if (pid < 0) {
		exit(EXIT_FAILURE);
	}
	
	// Success: Let the parent terminate 
	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}
	
	// On success: The child process becomes session leader 
	if (setsid() < 0) {
		exit(EXIT_FAILURE);
	}
	
	//Catch, ignore and handle signals 
	//TODO: Implement a working signal handler 
	//signal(SIGCHLD, SIG_IGN);
	//signal(SIGHUP, SIG_IGN);
	
	// Fork off for the second time
	pid = fork();
	
	// An error occurred 
	if (pid < 0) {
		exit(EXIT_FAILURE);
	}
	
	// Success: Let the parent terminate 
	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}
	
	// Set new file permissions 
	umask(0);

	// Close all open file descriptors 
	int x3;
	for (x3 = sysconf(_SC_OPEN_MAX); x3>=0; x3--)	{
		close (x3);
	}
#endif

	// ===========   open log file ==========
	if (logFile == 0) { //  try to open

		char p[512];
		if (getFolderLocation(p, 512)) {
			using namespace boost::filesystem;
			boost::system::error_code ec;
			path dir = path(p) / "log";
			path f = dir / "log.txt";

			if (!exists(dir, ec)) {
				bool logDirCreated = create_directories(dir, ec);
				if (!logDirCreated) {
					xm_printf("cannot create dirs [%s]; error [%s]\n", dir.string().c_str(),  ec.message().c_str());
					return 2;
				}
			}
			logFile = fopen(f.string().c_str(), "wt");
			if (logFile) {
#ifdef WIN32
				DWORD pid = GetCurrentProcessId();
#else
				pid_t pid = getpid(); //  my PID
#endif
				xm_printf("\n\n =========== %d =================\nlog started\n", pid);
			}
			else {
				return 3;
			}

		} else {
			printf("ERROR: cannot obtain log file location\n");
		}
	}
	
	/* Change the working directory to the root directory */
	/* or another appropriated directory */
	//chdir("/");

	int lockState = checkProcRunning();
	if (lockState != 0) {
		xm_printf("already started\n\n");
		exit(EXIT_FAILURE);
	}
	if (lockHandle() != 0) {
		xm_printf("cannot setup a lock\n\n");
		exit(EXIT_FAILURE);
	}

	/* Open the log file */
	//openlog ("firstdaemon", LOG_PID, LOG_DAEMON);

	//setbuf(stdout, NULL);
	//setvbuf(stdout, NULL, _IONBF, 0);
	xm_printf("main: starting \n");

	QApplication app(argc, argv);
	app.setQuitOnLastWindowClosed(false);   
	QWorker qWorker;
	qWorker.qtstart(false);
	QProcInterface qpi(qWorker, app);
	qpi.start();
	QObject::connect(&app, SIGNAL(aboutToQuit()), &qWorker, SLOT(onQtAppClosing()));

	QIcon icon;
    icon.addPixmap(QPixmap(QString::fromUtf8(":/icons/binokl.png")), QIcon::Normal, QIcon::Off);
	icon.addPixmap(QPixmap(QString::fromUtf8(":/icons/binokl.png")), QIcon::Normal, QIcon::On);
	app.setWindowIcon(icon);
	//QMainWindow::
	//qWorker.appMutex.lock();
	xm_printf("main: starting QT thread  \n");
	app.exec();
	xm_printf("main: QT thread finished  \n");
	//qWorker.appMutex.unlock();
	//qWorker.appV.notify_all();
	
	xm_printf("main: exiting \n");
	return 0;
}


void assert_failed(const char* file, unsigned int line, const char* str) {
	xm_printf("ASSERT faild: %s (file %s, line %d)\n", str, file, line);
}

static const int logBufLen = 2048;
static char logBuf[logBufLen];
int xm_printf(const char * _Format, ...) {
	va_list args;
	va_start(args, _Format);

	if (logFile == 0) {

	}

	int ok = vsnprintf(logBuf, logBufLen, _Format, args);
	logBuf[logBufLen - 1] = 0;
	if(ok > 0) { // we got the message
		if(logFile != 0) {
			fwrite(logBuf, 1, strlen(logBuf), logFile);
			fflush(logFile);
		}
		//std::cout << logBuf;
	}
	va_end(args);
	return 0;
}



