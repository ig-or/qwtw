



#include "qwproc.h"
#include "xstdef.h"
#include "qworker.h"
#include "qwproto.h"

#include <QObject>
#include <QPointer>
#include <QApplication>
#include <QIcon>
#include <QString>
#include <QMainWindow>
#include <QLibraryInfo>
#include <QVersionNumber>
#include <QtGlobal>

#include <stdlib.h>

#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef WIN32
	#include "windows.h"
	#include <shlobj.h>
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
int xmPrintLevel = 2; // will printf messages with level <= then this

int lockHandle() {
	using namespace boost::filesystem;
	boost::system::error_code ec;
	char p[512];
	if (getFolderLocation(p, 512)) {

	} else {
		xmprintf(0, "ERROR: getFolderLocation not working \n");
		return 3;
	}

	path home(p);
	path dir = home / "lock";
	path f = dir / "qwproc";

	if (!exists(dir, ec)) {
		bool lockDirCreated = create_directories(dir, ec);
		if (!lockDirCreated) {
			xmprintf(0, "cannot create dirs; error %s\n", ec.message().c_str());
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
	xmprintf(1, "lockHandle() complete\n");
	return 0;
}

int main(int argc, char** argv) {
	bool test = QProcInterface::runningAlready();
	if (test) {
		xmprintf(2, "shm exists\n");
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
					xmprintf(0, "cannot create dirs [%s]; error [%s]\n", dir.string().c_str(),  ec.message().c_str());
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
				xmprintf(0, "\n\n =========== %d =================\nlog started\n", pid);
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
		xmprintf(0, "already started\n\n");
		exit(EXIT_FAILURE);
	}
	if (lockHandle() != 0) {
		xmprintf(0, "cannot setup a lock\n\n");
		exit(EXIT_FAILURE);
	}

	/* Open the log file */
	//openlog ("firstdaemon", LOG_PID, LOG_DAEMON);

	//setbuf(stdout, NULL);
	//setvbuf(stdout, NULL, _IONBF, 0);
	xmprintf(1, "main: starting \n");
	QString l1 =  QLibraryInfo::location(QLibraryInfo::PluginsPath);
	QString l2 =  QLibraryInfo::location(QLibraryInfo::PrefixPath);
	QString l3 =  QLibraryInfo::location(QLibraryInfo::LibrariesPath);
	QString l4 =  QLibraryInfo::location(QLibraryInfo::LibraryExecutablesPath);
	std::string s1  =  l1.toStdString();
	std::string s2  =  l2.toStdString();
	std::string s3  =  l3.toStdString();
	std::string s4  =  l4.toStdString();
	#ifdef WIN32
	xmprintf(2, "\nWIN32 mode\n");
	#else
	xmprintf(2, "\nLINUX mode\n");
	#endif
	xmprintf(2, "QT: \nPluginsPath=%s\nPrefixPath=%s\nLibrariesPath=%s\nLibraryExecutablesPath=%s\n\n",
		s1.c_str(), s2.c_str(), s3.c_str(), s4.c_str());

	QString l5 = QCoreApplication::applicationDirPath();
	std::string s5 = l5.toStdString();
	xmprintf(2, "QT applicationDirPath = %s\n\n", s5.c_str());

	QVersionNumber qt_version = QLibraryInfo::version();
	QString l6 = qt_version.toString();
	std::string s6 = l6.toStdString();
	xmprintf(2, "QLibraryInfo::version(): %s \n", s6.c_str());

	const char* qv = qVersion();
	xmprintf(2, "qVersion() = %s \n", qv);
	xmprintf(2, "QT_VERSION_STR = %s \n", QT_VERSION_STR);


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
	xmprintf(2, "main: starting QT thread  \n");
	app.exec();
	xmprintf(2, "main: QT thread finished  \n");
	//qWorker.appMutex.unlock();
	//qWorker.appV.notify_all();
	
	xmprintf(2, "main: exiting \n");
	return 0;
}


void assert_failed(const char* file, unsigned int line, const char* str) {
	xmprintf(0, "ASSERT faild: %s (file %s, line %d)\n", str, file, line);
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

int xmprintf(int level, const char * _Format, ...) {
	if (level > xmPrintLevel) {
		return 1;
	}
	va_list args;
	va_start(args, _Format);

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



