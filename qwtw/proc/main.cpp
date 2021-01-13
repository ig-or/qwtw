



#include "qwproc.h"
#include "xstdef.h"
#include "qworker.h"
#include "qwproto.h"

#include <QObject>
#include <QPointer>
#include <QApplication>
#include <QIcon>
#include <QPixmap>
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

#ifdef USEMARBLE
#include <marble/MarbleDebug.h>
#include <marble/MarbleDirs.h>
#endif

#include "boost/filesystem.hpp"
#include <boost/program_options.hpp>

static QPointer<QWorker> q2worker(nullptr);
static FILE* logFile = 0;
int xmPrintLevel = 1; // will printf messages with level <= then this
int debugLevel = 0;

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
	boost::program_options::variables_map vm;
	boost::program_options::options_description desc(" Valid arguments");
	desc.add_options()
		("help", "This help message")		
		#ifdef USEMARBLE
		("marble_data", boost::program_options::value< std::string >(), "path to the Marble data files")
		("marble_plugins", boost::program_options::value< std::string >(), "path to the Marble plugins")
		#endif
		("debug",  boost::program_options::value< int >(), "debug level, 0 - minimum, 10 - maximum")
		("print",  boost::program_options::value< int >(), "print level, 0 - minimum, 10 - maximum")
		("second",  boost::program_options::value< int >(), "second")
		
	;
	try
	{
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc, 
			boost::program_options::command_line_style::unix_style ^
			boost::program_options::command_line_style::allow_short
			), vm);
		//boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
		boost::program_options::notify(vm);
	}
	catch(...) 	{
		std::cerr << "   : error: bad arguments" <<  std::endl << desc << std::endl << std::endl;
		std::cerr << "argc = " << argc << "; argv: ";
		for (int i = 0; i < argc; i++) {
			std::cerr << " " << argv[i] << ";";
		}
		std::cerr << std::endl;
		return 1;
	}
	//if(vm.count("marble_data"))  {
//		xmprintf(0, "@@@@@@@@@@ marble_data  =  %s\n", vm["marble_data"].as< std::string >().c_str());
	//}

	bool test = QProcInterface::runningAlready();
	if (test) {
		xmprintf(2, "BTW, shm already exists\n");
		//return 2;
	}
	
	if(vm.count("debug"))  {
		debugLevel = vm["debug"].as< int >();
		xmprintf(0, "using debug level from cmd: %d \n", debugLevel);
		if(vm.count("print"))  {
			xmPrintLevel = vm["print"].as< int >();
		} else {
			xmPrintLevel = debugLevel;
		}
	}

	try {
		std::ostringstream s;
		s << "MAIN argc = " << argc << "; argv: " << std::endl;	;
		for (int i = 0; i < argc; i++) {
			s << " [" << argv[i] << "] ";
		}
		s << std::endl;	
		xmprintf(0, "params: %s\n", s.str().c_str());
	} catch(...) {

	}

#ifdef WIN32	// make it a windows service could be a better way, though.. 
				//  probably this also works:

				
#else
	//  code partially from https://github.com/pasce/daemon-skeleton-linux-c:
	pid_t pid;
	if(vm.count("second"))  {

	} else {
		if (debugLevel == 0) {
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

			//std::list<std::string> args2;
			char** a = new char*[argc + 3]; 
			for (int i = 0; i < argc; i++) {
			//	args2.push_back(argv[i]);
				a[i] = argv[i];
			}

			//args2.push_back("--second");
			//args2.push_back("1");
			a[argc] = "--second";
			a[argc+1] = "1";
			a[argc+2] = NULL;
			execvp(a[0], a);
						

			//Catch, ignore and handle signals 
			//TODO: Implement a working signal handler 
			//signal(SIGCHLD, SIG_IGN);
			//signal(SIGHUP, SIG_IGN);
			
			// Fork off for the second time
			/*
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
			*/
		}
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
			}	else {
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

#ifdef USEMARBLE
	std::string mdp;
	std::string mpp;
	Marble::MarbleDebug::setEnabled( true );
	if(vm.count("marble_data"))  {
		mdp = vm["marble_data"].as< std::string >();
		xmprintf(0, "\tsetting  marble_data to [%s] \n", mdp.c_str());
		Marble::MarbleDirs::setMarbleDataPath(mdp.c_str());
	} else {
		xmprintf(0, "\tusing default marble_data \n");
	}

	if(vm.count("marble_plugins"))  {
		mpp = vm["marble_plugins"].as< std::string >();
		xmprintf(0, "\tsetting  marble_plugins to [%s] \n", mpp.c_str());
		Marble::MarbleDirs::setMarblePluginPath(mpp.c_str());
	} else {
		xmprintf(0, "\tusing default marble_plugins \n");
	}
	Marble::MarbleDirs::debug();
	if (debugLevel > 2) {
		Marble::MarbleDebug::setEnabled( true );
		xmprintf(0, "Marble debug enabled\n");
	} else {
		Marble::MarbleDebug::setEnabled( false );
	}

#endif

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

	QVersionNumber qt_version = QLibraryInfo::version();
	QString l6 = qt_version.toString();
	std::string s6 = l6.toStdString();
	xmprintf(2, "QLibraryInfo::version(): %s \n", s6.c_str());

	const char* qv = qVersion();
	xmprintf(2, "qVersion() = %s \n", qv);
	xmprintf(2, "QT_VERSION_STR = %s \n", QT_VERSION_STR);

	QApplication app(argc, argv);
	QString l5 = QCoreApplication::applicationDirPath();
	std::string s5 = l5.toStdString();
	xmprintf(2, "QT applicationDirPath = %s\n\n", s5.c_str());

	app.setQuitOnLastWindowClosed(false);   
	#ifdef USEMARBLE
	QWorker qWorker(mdp, mpp);
	#else
	QWorker qWorker;
	#endif
	qWorker.qtstart(false);
	QProcInterface qpi(qWorker, app);
	qpi.start();
	QObject::connect(&app, SIGNAL(aboutToQuit()), &qWorker, SLOT(onQtAppClosing()));

	QIcon icon;
    icon.addPixmap(QPixmap(QString::fromUtf8(":/icons/binokl.png")), QIcon::Normal, QIcon::Off);
	icon.addPixmap(QPixmap(QString::fromUtf8(":/icons/binokl.png")), QIcon::Normal, QIcon::On);
	app.setWindowIcon(icon);
	app.setWindowIcon(QIcon(":/icons/binokl.png"));
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

static std::list<std::string> xmb1List;
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
			if (xmb1List.size() > 0) {
				for (auto const& a : xmb1List) {
					fprintf(logFile, "%s", a.c_str());
				}
				xmb1List.clear();
			}
			//fwrite(logBuf, 1, strlen(logBuf), logFile);
			fprintf(logFile, "%d-%d \t%s", xmPrintLevel, level, logBuf);
			fflush(logFile);
		} else {
			if (xmb1List.size() < 10) {
				xmb1List.push_back(std::string(logBuf));
			}
		}
		//std::cout << logBuf;
	}
	va_end(args);
	return 0;
}



