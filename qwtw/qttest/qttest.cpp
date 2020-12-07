

//#include "xstdef.h"

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

#include "qttest.h"


QTTest::QTTest(QWidget * parent1): QDialog(parent1) {

}



int main(int argc, char** argv) {

	setbuf(stdout, NULL);
	setvbuf(stdout, NULL, _IONBF, 0);

	printf("main: starting \n");
	QString l1 =  QLibraryInfo::location(QLibraryInfo::PluginsPath);
	QString l2 =  QLibraryInfo::location(QLibraryInfo::PrefixPath);
	QString l3 =  QLibraryInfo::location(QLibraryInfo::LibrariesPath);
	QString l4 =  QLibraryInfo::location(QLibraryInfo::LibraryExecutablesPath);
	std::string s1  =  l1.toStdString();
	std::string s2  =  l2.toStdString();
	std::string s3  =  l3.toStdString();
	std::string s4  =  l4.toStdString();
	#ifdef WIN32
	printf("\nWIN32 mode\n");
	#else
	printf("\nLINUX mode\n");
	#endif
	printf("QT: \nPluginsPath=%s\nPrefixPath=%s\nLibrariesPath=%s\nLibraryExecutablesPath=%s\n\n",
		s1.c_str(), s2.c_str(), s3.c_str(), s4.c_str());

	QString l5 = QCoreApplication::applicationDirPath();
	std::string s5 = l5.toStdString();
	printf("QT applicationDirPath = %s\n\n", s5.c_str());

	QVersionNumber qt_version = QLibraryInfo::version();
	QString l6 = qt_version.toString();
	std::string s6 = l6.toStdString();
	printf("QLibraryInfo::version(): %s \n", s6.c_str());

	const char* qv = qVersion();
	printf("qVersion() = %s \n", qv);
	printf("QT_VERSION_STR = %s \n", QT_VERSION_STR);


	QApplication app(argc, argv);
	//app.setQuitOnLastWindowClosed(false);   
	QIcon icon;
    icon.addPixmap(QPixmap(QString::fromUtf8(":/icons/binokl.png")), QIcon::Normal, QIcon::Off);
	icon.addPixmap(QPixmap(QString::fromUtf8(":/icons/binokl.png")), QIcon::Normal, QIcon::On);
	app.setWindowIcon(icon);

	QTTest test;
	test.show();

	printf("main: starting QT thread  \n");
	app.exec();
	printf("main: QT thread finished  \n");

	return 0;
}


void assert_failed(const char* file, unsigned int line, const char* str) {
	printf("ASSERT faild: %s (file %s, line %d)\n", str, file, line);
}

