

//#include "xstdef.h"

#include <QObject>
#include <QPointer>
#include <QApplication>
#include <QIcon>
#include <QPixmap>
#include <QString>

#include <QMainWindow>
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
#ifdef USEMARBLE
#include <marble/MarbleDebug.h>
#include <marble/MarbleDirs.h>
#endif

#include "qttest.h"



#ifdef USEMARBLE
MWidgetEx::MWidgetEx(QWidget *parent): Marble::MarbleWidget(parent) {
	
	/*
	bool haveSSL = QSslSocket::supportsSsl();

	if (!haveSSL) {
		xm_printf("ERROR: looks like SSL not working; no maps will be downloaded\n");
		xm_printf("MWidgetEx ssl info: build version = %s, supports = %s,  lib version =  %s\n",
			QSslSocket::sslLibraryBuildVersionString().toUtf8().constData(),
			haveSSL ? "yes" : "no",
			QSslSocket::sslLibraryVersionString().toUtf8().constData());

	}
*/
}

void MWidgetEx::customPaint(Marble::GeoPainter* painter) {

}
void MWidgetEx::closeEvent(QCloseEvent * event) {

}


#endif

QTTest::QTTest(QWidget * parent1): QDialog(parent1, 
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


	#ifdef USEMARBLE
	Marble::MarbleDirs::debug();
	mw = new MWidgetEx(this);
	mw->setMapThemeId("earth/openstreetmap/openstreetmap.dgml");
	Marble::GeoDataDocument *document = new Marble::GeoDataDocument;
	mw->model()->treeModel()->addDocument(document);
	mw->setProjection(Marble::Mercator);
	mw->setShowBorders(true);
	mw->setShowGrid(true);
	mw->showGrid();
	mw->setShowCities(true);
	mw->setShowOverviewMap(false);
	mw->zoomView(DEFAULT_ZOOM_LEVEL);
	Marble::GeoDataCoordinates home(-122.00505316189623, 37.282928174000936, 10.0, Marble::GeoDataCoordinates::Degree);
	mw->centerOn(home);

	layout->addWidget(mw);

	#endif

	setLayout(layout);
	resize(400, 300);

	show();
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

	#ifdef USEMARBLE
	Marble::MarbleDebug::setEnabled( true );
	if(vm.count("marble_data"))  {
		std::string mdp = vm["marble_data"].as< std::string >();
		printf("\tserring  marble_data to [%s] \n", mdp.c_str());
		Marble::MarbleDirs::setMarbleDataPath(QString::fromStdString(mdp));
	} else {
		printf("\tusing default marble_data \n");
	}

	if(vm.count("marble_plugins"))  {
		std::string mpp = vm["marble_plugins"].as< std::string >();
		printf("\tserring  marble_plugins to [%s] \n", mpp.c_str());
		Marble::MarbleDirs::setMarblePluginPath(QString::fromStdString(mpp));
	} else {
		printf("\tusing default marble_plugins \n");
	}
	Marble::MarbleDirs::debug();
	#endif


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

