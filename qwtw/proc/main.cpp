#include "qwproc.h"
#include "xstdef.h"
#include "qworker.h"
#include "qwproto.h"

#include <QApplication>
#include <QIcon>
#include <QLibraryInfo>
#include <QMainWindow>
#include <QObject>
#include <QPixmap>
#include <QPointer>
#include <QString>
#include <QVersionNumber>
#include <QtGlobal>

#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef WIN32
	#include "windows.h"
	#include <shlobj.h>
#else
	#include <syslog.h>
	#include <unistd.h>
#endif

#ifdef USEMARBLE
#include <marble/MarbleDebug.h>
#include <marble/MarbleDirs.h>
#endif

static QPointer<QWorker> q2worker(nullptr);
static FILE* logFile = 0;
int xmPrintLevel = 1; // will printf messages with level <= than this
int debugLevel = 0;

namespace {

struct CliOptions {
	bool help = false;
	std::optional<int> debug;
	std::optional<int> print;
	std::optional<int> second;
#ifdef USEMARBLE
	std::optional<std::string> marbleData;
	std::optional<std::string> marblePlugins;
#endif
};

void printUsage(std::ostream& os) {
	os << " Valid arguments:\n"
	   << "  --help                   This help message\n"
#ifdef USEMARBLE
	   << "  --marble_data <path>     path to the Marble data files\n"
	   << "  --marble_plugins <path>  path to the Marble plugins\n"
#endif
	   << "  --debug <0..10>          debug level, 0 - minimum, 10 - maximum\n"
	   << "  --print <0..10>          print level, 0 - minimum, 10 - maximum\n"
	   << "  --second <int>           internal daemon flag\n";
}

bool parseIntValue(const std::string& s, int& out) {
	try {
		size_t idx = 0;
		int v = std::stoi(s, &idx, 10);
		if (idx != s.size()) {
			return false;
		}
		out = v;
		return true;
	} catch (...) {
		return false;
	}
}

bool parseCli(int argc, char** argv, CliOptions& opts) {
	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		auto nextVal = [&](std::string& val) -> bool {
			auto eq = arg.find('=');
			if (eq != std::string::npos) {
				val = arg.substr(eq + 1);
				return true;
			}
			if (i + 1 < argc) {
				val = argv[++i];
				return true;
			}
			return false;
		};

		std::string key = arg;
		auto eq = key.find('=');
		if (eq != std::string::npos) {
			key = key.substr(0, eq);
		}

		if (key == "--help" || key == "-h") {
			opts.help = true;
		} else if (key == "--debug") {
			std::string val;
			int parsed = 0;
			if (!nextVal(val) || !parseIntValue(val, parsed)) {
				return false;
			}
			opts.debug = parsed;
		} else if (key == "--print") {
			std::string val;
			int parsed = 0;
			if (!nextVal(val) || !parseIntValue(val, parsed)) {
				return false;
			}
			opts.print = parsed;
		} else if (key == "--second") {
			std::string val;
			int parsed = 0;
			if (!nextVal(val) || !parseIntValue(val, parsed)) {
				return false;
			}
			opts.second = parsed;
		}
#ifdef USEMARBLE
		else if (key == "--marble_data") {
			std::string val;
			if (!nextVal(val)) {
				return false;
			}
			opts.marbleData = val;
		} else if (key == "--marble_plugins") {
			std::string val;
			if (!nextVal(val)) {
				return false;
			}
			opts.marblePlugins = val;
		}
#endif
		// Ignore Qt platform arguments (e.g. -platform offscreen) passed through to QApplication
		else if (!key.empty() && key[0] == '-' && (key == "-platform" || key == "-style" || key == "-geometry")) {
			if (eq == std::string::npos && i + 1 < argc) {
				++i;
			}
		} else {
			return false;
		}
	}
	return true;
}

} // namespace

int main(int argc, char** argv) {
	CliOptions opts;
	if (!parseCli(argc, argv, opts)) {
		std::cout << "   : error: bad arguments\n";
		printUsage(std::cout);
		std::cout << "\nargc = " << argc << "; argv: ";
		for (int i = 0; i < argc; i++) {
			std::cout << " " << argv[i] << ";";
		}
		std::cout << std::endl;
		return 1;
	}

	if (opts.help) {
		std::cout << " (help)\n";
		printUsage(std::cout);
		return 0;
	}

	if (QProcInterface::runningAlready()) {
		xmprintf(2, "BTW, qwproc already running\n");
	}

	if (opts.debug.has_value()) {
		debugLevel = *opts.debug;
		xmprintf(0, "using debug level from cmd: %d\n", debugLevel);
		xmPrintLevel = opts.print.value_or(debugLevel);
	} else if (opts.print.has_value()) {
		xmPrintLevel = *opts.print;
	}

	{
		std::ostringstream s;
		s << "MAIN argc = " << argc << "; argv:\n";
		for (int i = 0; i < argc; i++) {
			s << " [" << argv[i] << "] ";
		}
		s << '\n';
		xmprintf(0, "params: %s\n", s.str().c_str());
	}

#ifndef WIN32
	if (!opts.second.has_value() && debugLevel == 0) {
		pid_t pid = fork();
		if (pid < 0) {
			exit(EXIT_FAILURE);
		}
		if (pid > 0) {
			exit(EXIT_SUCCESS);
		}
		if (setsid() < 0) {
			exit(EXIT_FAILURE);
		}

		std::vector<std::string> ownedArgs;
		ownedArgs.reserve(static_cast<size_t>(argc) + 2);
		for (int i = 0; i < argc; ++i) {
			ownedArgs.emplace_back(argv[i]);
		}
		ownedArgs.emplace_back("--second");
		ownedArgs.emplace_back("1");

		std::vector<char*> execArgs;
		execArgs.reserve(ownedArgs.size() + 1);
		for (auto& s : ownedArgs) {
			execArgs.push_back(s.data());
		}
		execArgs.push_back(nullptr);
		execvp(execArgs[0], execArgs.data());
	}
#endif

	// =========== open log file ==========
	if (logFile == 0) {
		char p[512];
		if (getFolderLocation(p, 512)) {
			std::error_code ec;
			std::filesystem::path dir = std::filesystem::path(p) / "log";
			std::filesystem::path f = dir / "log.txt";

			if (!std::filesystem::exists(dir, ec)) {
				bool logDirCreated = std::filesystem::create_directories(dir, ec);
				if (!logDirCreated && !std::filesystem::exists(dir, ec)) {
					xmprintf(0, "cannot create dirs [%s]; error [%s]\n", dir.string().c_str(), ec.message().c_str());
					return 2;
				}
			}
			logFile = fopen(f.string().c_str(), "wt");
			if (logFile) {
#ifdef WIN32
				DWORD pid = GetCurrentProcessId();
#else
				pid_t pid = getpid();
#endif
				xmprintf(0, "\n\n =========== %d =================\nlog started\n", pid);
			} else {
				return 3;
			}
		} else {
			printf("ERROR: cannot obtain log file location\n");
		}
	}

	if (QProcInterface::runningAlready()) {
		xmprintf(0, "already started!\n\n");
		exit(EXIT_SUCCESS);
	}

#ifdef USEMARBLE
	std::string mdp;
	std::string mpp;
	Marble::MarbleDebug::setEnabled(true);
	if (opts.marbleData.has_value()) {
		mdp = *opts.marbleData;
		xmprintf(0, "\tsetting marble_data to [%s]\n", mdp.c_str());
		Marble::MarbleDirs::setMarbleDataPath(QString::fromStdString(mdp));
	} else {
		xmprintf(0, "\tusing default marble_data\n");
	}

	if (opts.marblePlugins.has_value()) {
		mpp = *opts.marblePlugins;
		xmprintf(0, "\tsetting marble_plugins to [%s]\n", mpp.c_str());
		Marble::MarbleDirs::setMarblePluginPath(QString::fromStdString(mpp));
	} else {
		xmprintf(0, "\tusing default marble_plugins\n");
	}

	if (debugLevel > 2) {
		Marble::MarbleDebug::setEnabled(true);
		Marble::MarbleDirs::debug();
		xmprintf(0, "Marble debug enabled\n");
	} else {
		Marble::MarbleDebug::setEnabled(false);
	}
#endif

	xmprintf(1, "main: starting\n");
	std::string s1 = QLibraryInfo::path(QLibraryInfo::PluginsPath).toStdString();
	std::string s2 = QLibraryInfo::path(QLibraryInfo::PrefixPath).toStdString();
	std::string s3 = QLibraryInfo::path(QLibraryInfo::LibrariesPath).toStdString();
	std::string s4 = QLibraryInfo::path(QLibraryInfo::LibraryExecutablesPath).toStdString();
#ifdef WIN32
	xmprintf(2, "\nWIN32 mode\n");
#elif defined(__APPLE__)
	xmprintf(2, "\nMACOS mode\n");
#else
	xmprintf(2, "\nLINUX mode\n");
#endif
	xmprintf(2, "QT:\nPluginsPath=%s\nPrefixPath=%s\nLibrariesPath=%s\nLibraryExecutablesPath=%s\n\n",
		s1.c_str(), s2.c_str(), s3.c_str(), s4.c_str());

	QVersionNumber qt_version = QLibraryInfo::version();
	std::string s6 = qt_version.toString().toStdString();
	xmprintf(2, "QLibraryInfo::version(): %s\n", s6.c_str());
	xmprintf(2, "qVersion() = %s\n", qVersion());
	xmprintf(2, "QT_VERSION_STR = %s\n", QT_VERSION_STR);

	QApplication app(argc, argv);
	std::string s5 = QCoreApplication::applicationDirPath().toStdString();
	xmprintf(2, "QT applicationDirPath = %s\n\n", s5.c_str());

	app.setQuitOnLastWindowClosed(false);
#ifdef USEMARBLE
	QWorker qWorker(mdp, mpp);
#else
	QWorker qWorker;
#endif
	qWorker.qtstart(false);
	QProcInterface qpi(qWorker, app);
	if (!qpi.start()) {
		xmprintf(0, "main: failed to start IPC listener (another instance running?)\n");
		return 1;
	}
	QObject::connect(&app, &QCoreApplication::aboutToQuit, &qWorker, &QWorker::onQtAppClosing);

	QIcon icon;
	icon.addPixmap(QPixmap(QString::fromUtf8(":/icons/binokl.png")), QIcon::Normal, QIcon::Off);
	icon.addPixmap(QPixmap(QString::fromUtf8(":/icons/binokl.png")), QIcon::Normal, QIcon::On);
	app.setWindowIcon(icon);

	xmprintf(2, "main: starting QT thread\n");
	app.exec();
	xmprintf(2, "main: QT thread finished\n");

	xmprintf(2, "main: exiting\n");
	return 0;
}

void assert_failed(const char* file, unsigned int line, const char* str) {
	xmprintf(0, "ASSERT faild: %s (file %s, line %d)\n", str, file, line);
}

static const int logBufLen = 2048;
static char logBuf[logBufLen];
int xm_printf(const char* _Format, ...) {
	va_list args;
	va_start(args, _Format);

	int ok = vsnprintf(logBuf, logBufLen, _Format, args);
	logBuf[logBufLen - 1] = 0;
	if (ok > 0 && logFile != 0) {
		fwrite(logBuf, 1, strlen(logBuf), logFile);
		fflush(logFile);
	}
	va_end(args);
	return 0;
}

static std::list<std::string> xmb1List;
int xmprintf(int level, const char* _Format, ...) {
	if (level > xmPrintLevel) {
		return 1;
	}
	va_list args;
	va_start(args, _Format);

	int ok = vsnprintf(logBuf, logBufLen, _Format, args);
	logBuf[logBufLen - 1] = 0;
	if (ok > 0) {
		if (logFile != 0) {
			if (!xmb1List.empty()) {
				for (auto const& a : xmb1List) {
					fprintf(logFile, "%s", a.c_str());
				}
				xmb1List.clear();
			}
			fprintf(logFile, "%d-%d \t%s", xmPrintLevel, level, logBuf);
			fflush(logFile);
		} else if (xmb1List.size() < 10) {
			xmb1List.emplace_back(logBuf);
		}
	}
	va_end(args);
	return 0;
}
