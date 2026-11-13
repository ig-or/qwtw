#include <QApplication>
#include <QCommandLineOption>
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

#ifdef USEMARBLE
#include <marble/MarbleDebug.h>
#include <marble/MarbleDirs.h>
#endif

#include "qttest.h"

#ifdef USEMARBLE
MWidgetEx::MWidgetEx(QWidget* parent) : Marble::MarbleWidget(parent) {}

void MWidgetEx::customPaint(Marble::GeoPainter* painter) {
	(void)painter;
}
void MWidgetEx::closeEvent(QCloseEvent* event) {
	(void)event;
}
#endif

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

#ifdef USEMARBLE
	Marble::MarbleDirs::debug();
	mw = new MWidgetEx(this);
	mw->setMapThemeId("earth/openstreetmap/openstreetmap.dgml");
	Marble::GeoDataDocument* document = new Marble::GeoDataDocument;
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
#ifdef USEMARBLE
	QCommandLineOption marbleDataOpt("marble_data", "path to the Marble data files", "path");
	QCommandLineOption marblePluginsOpt("marble_plugins", "path to the Marble plugins", "path");
	parser.addOption(marbleDataOpt);
	parser.addOption(marblePluginsOpt);
#endif
	parser.process(app);

	std::string s5 = QCoreApplication::applicationDirPath().toStdString();
	printf("QT applicationDirPath = %s\n\n", s5.c_str());

	QIcon icon;
	icon.addPixmap(QPixmap(QString::fromUtf8(":/icons/binokl.png")), QIcon::Normal, QIcon::Off);
	icon.addPixmap(QPixmap(QString::fromUtf8(":/icons/binokl.png")), QIcon::Normal, QIcon::On);
	app.setWindowIcon(icon);

#ifdef USEMARBLE
	Marble::MarbleDebug::setEnabled(true);
	if (parser.isSet(marbleDataOpt)) {
		QString mdp = parser.value(marbleDataOpt);
		printf("\tsetting marble_data to [%s]\n", mdp.toStdString().c_str());
		Marble::MarbleDirs::setMarbleDataPath(mdp);
	} else {
		printf("\tusing default marble_data\n");
	}

	if (parser.isSet(marblePluginsOpt)) {
		QString mpp = parser.value(marblePluginsOpt);
		printf("\tsetting marble_plugins to [%s]\n", mpp.toStdString().c_str());
		Marble::MarbleDirs::setMarblePluginPath(mpp);
	} else {
		printf("\tusing default marble_plugins\n");
	}
	Marble::MarbleDirs::debug();
#endif

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
