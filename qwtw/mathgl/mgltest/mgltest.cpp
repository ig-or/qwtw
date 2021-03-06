

//#include "xstdef.h"

#include <QObject>
#include <QPointer>
#include <QApplication>
#include <QIcon>
#include <QTimer>
#include <QPixmap>
#include <QString>

//#include <QMainWindow>
#include <QLibraryInfo>
#include <QVersionNumber>

#include <QVBoxLayout>
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

#include <boost/program_options.hpp>

#include <string>
#include <iostream>
#include <sstream>

#include "qmgl.h"
#include "mgltest.h"
#include "line.h"
#include "qwtypes.h"


LineItemInfo* ii1 = 0;
LineItemInfo* ii2 = 0;

MGLTest::MGLTest(QWidget * parent1): QDialog(parent1, 
		Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint | 
		Qt::WindowCloseButtonHint | Qt::WindowContextHelpButtonHint) {

	QIcon icon;

	QPixmap pm(":/icons/binokl.png");
	printf("main wnd QPixmap: %s \n", pm.isNull() ? "null" : "OK");
    icon.addPixmap(QPixmap(QString::fromUtf8(":/icons/binokl.png")), QIcon::Normal, QIcon::Off);
	icon.addPixmap(QPixmap(QString::fromUtf8(":/icons/binokl.png")), QIcon::Normal, QIcon::On);

	printf("main wnd icon: %s \n", icon.isNull() ? "null" : "OK");
    setWindowIcon(icon);

	QVBoxLayout *layout = new QVBoxLayout;
	layout->setSpacing(2); layout->setMargin(2);

	test1 = new QMGL1(this);
	//test12 = new QMGL1(this);
	//test2 = new QMGL2(this);
	layout->addWidget(test1);
	//layout->addWidget(test12);
	setLayout(layout);
	resize(800, 600);

	QTimer::singleShot(500, this, &MGLTest::test);

	show();
}

void MGLTest::test() {
	printf("test\n");

	const int N = 100;
	double x[N];
	double y[N];
	double z[N];
	double tt[N];
	double t;
	double R = 10.0;
	double x1[N];
	double y1[N];
	double z1[N];


	for (int i = 0; i < N; i++) {
		t = (double(i) / double(N)) * 2. * 3.14159 * 3.5;
		tt[i] = t;
		x[i] = R * sin(t);
		y[i] = R * cos(t);
		z[i] = t;

		x1[i] = R * sin(t);
		y1[i] = R * cos(t);
		z1[i] = x1[i] + y1[i];
	}
	ii1 = new LineItemInfo(x, y, z, N, "first line", tt);     ii1->style = "-sb";
	ii2 = new LineItemInfo(x1, y1, z1, N, "second line", tt);   ii2->style = "-or";


	constexpr int xSize = 20;
	constexpr int ySize = 20;
	double xMin = -10., xMax = 10.0, yMin = -10.0, yMax = 10.0;
	double f[xSize * ySize];
	double ax, ay;
	for (int i = 0; i < xSize; i++) {
		ax = xMin + i;
		for (int j = 0; j < ySize; j++) {
			ay = yMin + j;
			f[i + xSize * j] = sin(ax / 5.0) * sin(ay / 7.0) * 10.0;
		}
	}

	

	//test1->addLine(N, x, y, z, "-sb");
	//test1->addLine(N, x1, y1, z1, "-or");
	test1->addLine(ii1);
	test1->addLine(ii2);

	//return;

	test1->addSurf(MeshInfo{ xSize, ySize, xMin, xMax, yMin, yMax, f, "", "", sdMesh });
	//test12->addLine(N, x, y, z, "-om");

}



int main(int argc, char** argv) {

	boost::program_options::variables_map vm;
	boost::program_options::options_description desc(" Valid arguments");
	desc.add_options()
		("help", "This help message")		
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



	QVersionNumber qt_version = QLibraryInfo::version();
	QString l6 = qt_version.toString();
	std::string s6 = l6.toStdString();
	printf("QLibraryInfo::version(): %s \n", s6.c_str());

	const char* qv = qVersion();
	printf("qVersion() = %s \n", qv);
	printf("QT_VERSION_STR = %s \n", QT_VERSION_STR);


	QApplication app(argc, argv);

	QString l5 = QCoreApplication::applicationDirPath();
	std::string s5 = l5.toStdString();
	printf("QT applicationDirPath = %s\n\n", s5.c_str());

	//app.setQuitOnLastWindowClosed(false);   
	QIcon icon;
    icon.addPixmap(QPixmap(QString::fromUtf8(":/icons/binokl.png")), QIcon::Normal, QIcon::Off);
	icon.addPixmap(QPixmap(QString::fromUtf8(":/icons/binokl.png")), QIcon::Normal, QIcon::On);
	app.setWindowIcon(icon);
	app.setWindowIcon(QIcon(":/icons/binokl.png"));

	MGLTest test;
	test.show();

	printf("main: starting QT thread  \n");
	app.exec();
	printf("main: QT thread finished  \n");

	return 0;
}


void assert_failed(const char* file, unsigned int line, const char* str) {
	printf("ASSERT faild: %s (file %s, line %d)\n", str, file, line);
}

static const int logBufLen = 2048;
static char logBuf[logBufLen];
int xmprintf(int level, const char * _Format, ...) {
	va_list args;
	va_start(args, _Format);

	int ok = vsnprintf(logBuf, logBufLen, _Format, args);
	logBuf[logBufLen - 1] = 0;
	if(ok > 0) { // we got the message
		std::cout << logBuf;
	}
	va_end(args);
	return 0;
}
