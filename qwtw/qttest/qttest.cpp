#include <QApplication>
#include <QCommandLineParser>
#include <QIcon>
#include <QLibraryInfo>
#include <QMainWindow>
#include <QObject>
#include <QPixmap>
#include <QPointer>
#include <QString>
#include <QVBoxLayout>
#include <QVersionNumber>
#include <QtGlobal>

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

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

#include "qttest.h"

QTTest::QTTest(QWidget* parent1)
	: QDialog(parent1,
		Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint |
		Qt::WindowCloseButtonHint | Qt::WindowContextHelpButtonHint) {

	QIcon icon;
	QPixmap pm(":/icons/binokl.png");
	printf("main wnd QPixmap: %s\n", pm.isNull() ? "null" : "OK");
	icon.addPixmap(QPixmap(QString::fromUtf8(":/icons/binokl.png")), QIcon::Normal, QIcon::Off);
	icon.addPixmap(QPixmap(QString::fromUtf8(":/icons/binokl.png")), QIcon::Normal, QIcon::On);

	printf("main wnd icon: %s\n", icon.isNull() ? "null" : "OK");
	setWindowIcon(icon);

	QVBoxLayout* layout = new QVBoxLayout;
	layout->setSpacing(2);
	layout->setContentsMargins(2, 2, 2, 2);

	setLayout(layout);
	resize(400, 300);
	show();
}

int main(int argc, char** argv) {
	setbuf(stdout, NULL);
	setvbuf(stdout, NULL, _IONBF, 0);

	printf("main: starting\n");
	std::string s1 = QLibraryInfo::path(QLibraryInfo::PluginsPath).toStdString();
	std::string s2 = QLibraryInfo::path(QLibraryInfo::PrefixPath).toStdString();
	std::string s3 = QLibraryInfo::path(QLibraryInfo::LibrariesPath).toStdString();
	std::string s4 = QLibraryInfo::path(QLibraryInfo::LibraryExecutablesPath).toStdString();
#ifdef WIN32
	printf("\nWIN32 mode\n");
#elif defined(__APPLE__)
	printf("\nMACOS mode\n");
#else
	printf("\nLINUX mode\n");
#endif
	printf("QT:\nPluginsPath=%s\nPrefixPath=%s\nLibrariesPath=%s\nLibraryExecutablesPath=%s\n\n",
		s1.c_str(), s2.c_str(), s3.c_str(), s4.c_str());

	QVersionNumber qt_version = QLibraryInfo::version();
	std::string s6 = qt_version.toString().toStdString();
	printf("QLibraryInfo::version(): %s\n", s6.c_str());
	printf("qVersion() = %s\n", qVersion());
	printf("QT_VERSION_STR = %s\n", QT_VERSION_STR);

	QApplication app(argc, argv);

	QCommandLineParser parser;
	parser.addHelpOption();
	parser.process(app);

	std::string s5 = QCoreApplication::applicationDirPath().toStdString();
	printf("QT applicationDirPath = %s\n\n", s5.c_str());

	QIcon icon;
	icon.addPixmap(QPixmap(QString::fromUtf8(":/icons/binokl.png")), QIcon::Normal, QIcon::Off);
	icon.addPixmap(QPixmap(QString::fromUtf8(":/icons/binokl.png")), QIcon::Normal, QIcon::On);
	app.setWindowIcon(icon);

	QTTest test;
	test.show();

	printf("main: starting QT thread\n");
	app.exec();
	printf("main: QT thread finished\n");

	return 0;
}

void assert_failed(const char* file, unsigned int line, const char* str) {
	printf("ASSERT faild: %s (file %s, line %d)\n", str, file, line);
}
