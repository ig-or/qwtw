
/**

	2D and 3D line views container.
	

	\file sfigure.cpp
	\author   Igor Sandler
	\date    Jul 2009
	\version 1.0
	
*/

#ifdef ENABLE_UDP_SYNC

#include <boost/asio.hpp>
#include <boost/array.hpp>
#endif

#include "sfigure.h"
#include "figure2.h"
#include <sstream>

#include "justaplot.h"
#include "xstdef.h"
//#include <QApplication>
#include <QDialog>
#include <QTimer>
#include <chrono>
//#include <QWindow>
//#include <QtPlatformHeaders/QWindowsWindowFunctions>

#include "qdesktopwidget.h"
#include "xmatrixplatform.h"
#include "xmatrix2.h"
#include "qwtypes.h"
//#include "topviewplot.h"

#ifdef USEMARBLE
#include "marbleview.h"
#endif
#ifdef USE_QT3D
#include "qt-3d.h"
#endif
#include "line.h"
#ifdef USEMATHGL
#include "qmglplot.h"
#include "qmgl.h"
#include <mgl2/qmathgl.h>
#endif


int xmprintf(int level, const char * _Format, ...);

#include <iostream>
#include <atomic>
#include <condition_variable>
//#include <thread>
//#include <chrono>
#include <boost/chrono.hpp>
#include <functional>
//#include <mutex>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <stdexcept>
#include <boost/bind.hpp>
#include "xmutils.h"
#include "settings.h"


#ifdef ENABLE_UDP_SYNC
using boost::asio::ip::udp; 

#pragma pack(1)
struct BroadcastMessage {
	char head[4];
	double pos[3];
	char tail[4];
};

///   UDP message about the picker info
struct PickerMessage {
	char head[4];
	CBPickerInfo cpi;
	char tail[4];
};

/**
 UDP message about 'clip'
*/
struct ClipUdpMessage {
	char head[4]; ///< "CCCC" ?
	double t1;   ///< time 1
	double t2;  ///< time 2
	int clipGroup = 0;    ///< clip group
	int havePos = 0;   ///< 1 if pos1 and pos2 are valid
	///  if we have some 3D line corresponding to all this,
	///  then this will be the point closest to 't1'
	double pos1[3];    
	double pos2[3];
	char tail[4];
};

#pragma pack()
class BCUdpClient {
public:
	BCUdpClient() : resolver(io_context), socket(io_context) {
		ok = false;
		int port = qwSettings.udp_client_port;
		try {
			destination = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), port);
		}	catch (const std::exception& ex) {
			xmprintf(0, "ERROR: BCUdpClient(): cannot create UDP endpoint on port %d (%s)\n", port, ex.what());
			return;
		}

		//receiver_endpoint = *resolver.resolve(q);
		try {
			//socket.set_option(udp::socket::broadcast)
			socket.open(udp::v4());
			socket.set_option(udp::socket::reuse_address(true));
		}  catch (const std::exception& ex) {
			xmprintf(0, "ERROR: BCUdpClient(): cannot open UDP port %d (%s)\n", port, ex.what());
			return;
		}

		ok = true;
		xmprintf(5, "BCUdpClient() created;  UDP port %d \n", port);
	}

	void bcSend(unsigned char* buf, int size) {
		if (!ok) {
			return;
		}
		try {
			size_t bs = socket.send_to(boost::asio::buffer(buf, size), destination);
			//xmprintf(9, "bcSend %d bytes \n", bs);
		} catch (const std::exception& ex) {
			xmprintf(1, "bcSend: exception: %s\n", ex.what());
		}
	}

private: 
	boost::asio::io_context io_context;
	udp::resolver resolver;
	boost::asio::ip::udp::endpoint destination;
	udp::endpoint receiver_endpoint;
	udp::socket socket;
	bool ok;
};

class BCUdpServer {

private:
	volatile bool created, createdMarker, somethingWasChanged;
	volatile int cmd;
	double pos[3];
	boost::mutex mu;
	//std::thread st;
	boost::thread st;
	volatile bool pleaseStop;
	boost::asio::io_context io_context;
	udp::socket socket_;
	udp::endpoint remote_endpoint_;
	int portNumber;
	unsigned char	rb[256];
	std::function<void(double[3])> onPointF;

public:
	BCUdpServer(int port) : portNumber(port), socket_(io_context, udp::endpoint(udp::v4(), port)) {
		created = false;
		createdMarker = false;
		somethingWasChanged = false;
		cmd = 0;
	}

	void bStart(std::function<void(double[3])> aPoint) {
		onPointF = aPoint;
		pleaseStop = false;
		//std::thread tr([&] { tcpThread(); });
		boost::thread tr([&] { tcpThread(); });
		st.swap(tr);
		created = true;
	}

	~BCUdpServer() {
		pleaseStop = true;
		using std::chrono::system_clock;
		using namespace std::chrono_literals;
		if (!created) {
			return;
		}
		// wait for the task to finish??
		boost::asio::io_context io_service1;
		udp::socket s1(io_service1);
		s1.open(udp::v4());
		unsigned char b[5];
		boost::asio::ip::udp::endpoint destination = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), portNumber);
		try {
			s1.send_to(boost::asio::buffer(b, 5), destination);
			s1.send_to(boost::asio::buffer(b, 5), destination);
			s1.send_to(boost::asio::buffer(b, 5), destination);
			//Sleep(10);
			//std::this_thread::sleep_for(10ms);
			boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
			s1.send_to(boost::asio::buffer(b, 5), destination);
		} catch (const std::exception& ex) {
			xmprintf(2, "exception: %s\n", ex.what());
		}

		st.join();
		int itmp = 0;
	}
	void tcpThread() {
		try {
			start_receive();
			xmprintf(2, "BCUdpServer tcpThread started; port %d\n", qwSettings.udp_server_port);
			io_context.run();
		} catch (std::exception& e) {
			std::cerr << e.what() << std::endl;
			xm_printf("TRACE: RDFramerDebugGuiUpdateCallback  exception: %s  \n", e.what());
		}
		return;
	}
	void start_receive() {
		socket_.async_receive_from(
			boost::asio::buffer(rb), remote_endpoint_,
			boost::bind(&BCUdpServer::handle_receive, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}

	void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred) {
		if (!error || error == boost::asio::error::message_size) {
			xmprintf(9, "UDP server handle_receive: bytes_transferred = %d \n", bytes_transferred);
			
			if ((bytes_transferred == 4) && ((memcmp(rb, "XXXX", 4) == 0))) {
				mu.lock();
				cmd = 2;
				somethingWasChanged = true;
				mu.unlock();
			}

			if (bytes_transferred == 8 * 3 + 4) {
				if ((memcmp(rb, "EEEE", 4) == 0) || ((memcmp(rb + 4 + 3 * 8, "FFFF", 4) == 0))) {
					rb[25] = 0;
					xm_printf("TRACE: RDFramerDebugGuiUpdateCallback: got %s \n", rb);
				} else {
					if (memcmp(rb, "CRDS", 4) == 0) {  //  message from Collage?
						mu.lock();
						memcpy(pos, rb + 4, 3 * 8);
						cmd = 1;
						somethingWasChanged = true;
						onPointF(pos);
						mu.unlock();
						xmprintf(9, "UDP server handle_receive: got CRDS message \n");
					}		else {
						xm_printf("TRACE: RDFramerDebugGuiUpdateCallback: got %s \n", rb);
					}
				}
			}

			if (pleaseStop) {
				return;
			}
			start_receive();
		}
	}
};



#endif

static unsigned int linesHistoryCounter = 0;

XQPlots::XQPlots(QWidget * parent1): /*QMainWindow(parent1,   // */QDialog(parent1, Qt::Dialog |
		Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint | 
		Qt::WindowCloseButtonHint | Qt::WindowContextHelpButtonHint) {
	parent = parent1;
	cf = 0;  //cf3 = 0;
	currentClipGroup = 0;
	markersAreVisible = false;
	//currentFigureMode = 2;  //  
	clearingAllFigures = false;
	currentImportanceMode = true;
	//memset(ecefOrigin, 0, 3*sizeof(double));  //  this means 'not set'
	ui.setupUi(this);

	QIcon icon;
    icon.addPixmap(QPixmap(QString::fromUtf8(":/icons/binokl.png")), QIcon::Normal, QIcon::Off);
	icon.addPixmap(QPixmap(QString::fromUtf8(":/icons/binokl.png")), QIcon::Normal, QIcon::On);
    setWindowIcon(icon);
	//setIcon(icon);

	//QWindow w;
	//QWindowsWindowFunctions::


	/*QStandardItem *i0 = pim.invisibleRootItem();
	i0->insertColumn(0, new QStandardItem("key"));

	for (int i = 0; i < 4; ++i) {
		QStandardItem *item = new QStandardItem(QString("item %0").arg(i));
		i0->appendRow(item);
		i0 = item;
	}
	*/
	ui.tv->setModel(&pim);
	connect(ui.tv, SIGNAL(clicked(QModelIndex)), this, SLOT(onTvItemClicked(QModelIndex)));

	connect(ui.tbClosePlots, SIGNAL(clicked(bool)), this, SLOT(onCloseAllPlots(bool)));
	connect(ui.tbShowEverything, SIGNAL(clicked(bool)), this, SLOT(onShowAllPlots(bool)));
	connect(ui.tbShowSimple, SIGNAL(clicked(bool)), this, SLOT(onShowAllSimple(bool)));
#ifdef ENABLE_UDP_SYNC
	broadCastInfo = 0;
	//bc = 0;
	///  start UDP client 
	// since we'd send out picker info anyway
	bc = new BCUdpClient();
	bServer = 0;

	//  start picker info filter thread
	std::thread ttmp = std::thread(&XQPlots::pFilterThreadF, this);
	pFilterThread.swap(ttmp);
#endif
	QTimer::singleShot(500, this, &XQPlots::onTest);
}

XQPlots::~XQPlots() {
	xm_printf("stopping XQPlots .... \n");
	onExit();
	xm_printf("XQPlots stopped\n");
}

void XQPlots::setmode(int mode_) {
    //currentFigureMode = mode_;
}

void XQPlots::setImportant(bool i) {
	currentImportanceMode = i;
}

void XQPlots::clear() {
	clearFigures();
}

void XQPlots::onTest() {
	//xmprintf(0, "XQPlots::onTest() 123\n");
	//printf("XQPlots::onTest() 123\n");

	//QMGL1* qmgl = new QMGL1(this);
	//QMathGL* mgl = new QMathGL(this);
	//xmprintf(0, "XQPlots::onTest() OK\n");
	//printf(0, "XQPlots::onTest() OK\n");
}

void XQPlots::showMainWindow() {
	show();

	activateWindow();
	raise();
	//showMaximized();
	showNormal();
}

JustAplot* XQPlots::figure(std::string name_, JPType type, unsigned int flags){
	std::map<std::string,  JustAplot*>::iterator it = figures.find(name_);

	xmprintf(5, "XQPlots::figure start \n");
	if (it != figures.end()) {
		cf = it->second;
		cf->activateWindow();
		cf->raise();
		//cf->showMaximized();
		cf->showNormal();

	}   else  {

		switch(type) {
		case jQWT:
			cf	= new Figure2(name_, this, parent, flags);
			break;
#ifdef USEMARBLE
		case jMarble: {
				MarView* tvp = new MarView(name_, this, parent);
				tvp->mvInit();
				cf = tvp;
			}
			break;
#endif
#ifdef USE_QT3D
		case jQT3D: {
			Q3DView* q3 = new Q3DView(name_, this, parent);
			q3->q3Init();
			cf = q3;
		}
			break;
#endif
#ifdef USEMATHGL
		case jMathGL: {
				xmprintf(5, "XQPlots::figure creating QMglPlot.. \n");
				QMglPlot* q4 = new QMglPlot(name_, this, parent);
				xmprintf(5, "XQPlots::figure  qInit.. \n");
				q4->qInit();
				xmprintf(5, "XQPlots::figure creating QMglPlot done\n");
				cf = q4;
			}
#endif
		};
		bool test = true;
		cf->clipGroup = currentClipGroup;
		test = connect(cf, SIGNAL(exiting (const std::string&)), this, SLOT(onFigureClosed(const std::string&)));
		test = connect(cf, SIGNAL(onSelection(const std::string&)), this, SLOT(onSelection(const std::string&)));
		test = connect(cf, SIGNAL(onPicker(const std::string&, double, double)), this, SLOT(onPicker(const std::string&, double, double)));

		figures.insert(make_pair(name_, cf));
		cf->show();

		// add to tree view:
		//ui.tv->iie

		static const QColor iColors[5] = { Qt::yellow, Qt::green,	Qt::lightGray,	Qt::blue,	Qt::darkYellow };
		static const QColor jColors[5] = { Qt::black, Qt::black,	Qt::black,		Qt::white,	Qt::blue };

		QStandardItem *i0 = pim.invisibleRootItem();
		QList<QStandardItem *> raw; 

		QStandardItem* kim = new QStandardItem(cf->key.c_str());
		raw.append(kim);
		raw.append(new QStandardItem(cf->name.c_str()));

		QStandardItem* gn = new QStandardItem(std::to_string(cf->clipGroup).c_str());
		raw.append(gn);
		QBrush br;   QColor clr(iColors[cf->clipGroup % 5]);
		br.setColor(clr);

		gn->setBackground(br);
		gn->setForeground(jColors[cf->clipGroup % 5]);

		QFont serifFont("Times", 12, QFont::Bold);
		gn->setFont(serifFont);
		i0->appendRow(raw);
	}
	xmprintf(5, "XQPlots::figure finish \n");
	return cf;
}

void XQPlots::onTvItemClicked(QModelIndex mi) {
	QStandardItem *i = pim.itemFromIndex(mi);
	int row = i->row();
	QStandardItem *j =  pim.item(row, 1);

	if (j == 0) { //  error?
		return;
	}
	std::string id = j->text().toUtf8().toStdString();
	std::map<std::string, JustAplot*>::iterator it = figures.find(id);

	if (it == figures.end()) { // error?
		return;
	}
		
	if (!callbackWorking) {
		cf = it->second;
		cf->activateWindow();
		cf->raise();
		//cf->showMaximized();
		cf->showNormal();
	}

	/*
	JustAplot* p = getPlotByName(i->text().toUtf8().toStdString());
	if (p != 0) {
		p->activateWindow();
		p->raise();
				//p->showMaximized();
		p->showNormal();
	}
	*/
}

JustAplot* XQPlots::getPlotByName(std::string s) {
	JustAplot* ret = 0;

	std::map<std::string, JustAplot*>::iterator it;
	for (it = figures.begin(); it != figures.end(); it++) {
		if (it->second->name.compare(s) == 0) {
			ret = it->second;
			break;
		}
	}
	return ret;
}

void XQPlots::onCloseAllPlots(bool checked) {
	clear();
}

void XQPlots::onShowAllSimple(bool checked) {
	for (FSet::iterator it = figures.begin(); it != figures.end(); it++) {
		it->second->show();
		it->second->raise();
	}
}

void XQPlots::onShowAllPlots(bool checked) {
	int n = figures.size();
	if (n < 1) return;
	double side = sqrt(n);
	int w = ceil(side);
	int h = floor(side);
	if (w*h < n) {
		h++;
	}
	mxat(w*h >= n);
	// /workspace/srcdir/qwtw/qwtw/sfigure.cpp:404:54: warning: 
	//‘const QRect QDesktopWidget::screenGeometry(int) const’ is deprecated: 
	// Use QGuiApplication::screens() [-Wdeprecated-declarations]
	QRect rec = QApplication::desktop()->screenGeometry();
	int height = rec.height();
	int width = rec.width() - 48;
	int hh = floor(height / h);
	int ww = floor(width / w);
	int i = 0, j = 0;
	QRect g, g1, g2;
	int dx, dy;

	for (FSet::iterator it = figures.begin(); it != figures.end(); it++) {
		g1 = it->second->frameGeometry();
		g2 = it->second->geometry();
		dx = g1.width() - g2.width();
		dy = g1.height() - g2.height();

		
		it->second->showNormal();
		it->second->resize(ww - dx, hh - dy);
		it->second->move(i*ww, j*hh);

		/*g.setTop(j*hh); 
		g.setBottom((j + 1)*hh);
		g.setLeft(i*ww);
		g.setRight((i + 1)*ww);
		*/
		
		//it->second->setGeometry(g);
		
		it->second->show();
		it->second->raise();
		
		i++;
		if (i >= w) {
			i = 0;
			j++;
		}
	}

}

void XQPlots::drawMarker(const std::string& key_, double X, double Y, int type) {
	//if (!markersAreVisible) markersAreVisible = true;
	std::map<std::string,  JustAplot*>::iterator it = figures.find(key_);
	if (it != figures.end()) {
		it->second->drawMarker(X, Y, type);
	}
}

void XQPlots::setAllMarkersVisible(bool visible) {
	std::map<std::string, JustAplot*>::iterator it;
	for (it = figures.begin(); it != figures.end(); it++) {
		it->second->makeMarkersVisible(visible);
	}
}

void XQPlots::on3DMarker(double p[3]) {
	int i = 0;
	V3 point(p), p1;
	// lets find the timestamp:
	if (broadCastInfo != 0) {
		int n = broadCastInfo->smallCoordIndex.size();
		if (n > 1) {
			unsigned long index = 0;
			double d = BIGNUMBER;
			for (unsigned int ix :  broadCastInfo->smallCoordIndex) {
				p1[0] = broadCastInfo->x[ix];
				p1[1] = broadCastInfo->y[ix];
				p1[2] = broadCastInfo->z[ix];
				double d1 = (point - p1).norma2();
				if (d1 < d) {
					d = d1;
					index = ix;
				}
			}
			double time = broadCastInfo->time[index];
			//QMetaObject::invokeMethod(this, "drawAllMarkers", Qt::QueuedConnection, Q_ARG(double, time));
			bool test  =  QMetaObject::invokeMethod(this, "drawAllMarkers1", Qt::QueuedConnection, 
				Q_ARG(int, index),
				Q_ARG(double, broadCastInfo->x[index]), Q_ARG(double, broadCastInfo->y[index]), Q_ARG(double, broadCastInfo->z[index]),
				Q_ARG(double, time));
			if (!test) {
				xmprintf(2, "XQPlots::on3DMarker() drawAllMarkers1 failed \n");
			}
		}
	}
}

Q_INVOKABLE void XQPlots::addVMarkerEverywhere(double t, const char* label, int id_, JustAplot* p) {
    std::map<std::string, JustAplot*>::iterator it;
    for(it = figures.begin(); it != figures.end(); it++) {
		if (p && (p->clipGroup != it->second->clipGroup)) {
			continue;
		}
		if (it->second->looksLikeTopView()) {
			continue;
		}
		it->second->addVMarker(t, label, id_);
		it->second->replot();
    }
}
Q_INVOKABLE void XQPlots::removeVMarkerEverywhere(int id_) {
    std::map<std::string, JustAplot*>::iterator it;
    for(it = figures.begin(); it != figures.end(); it++) {
	   it->second->removeVMarker(id_);
	   it->second->replot();
    }
}

void XQPlots::setUdpCallback(OnUdpCallback  cb) {
	onUdpCallback = cb;
}
void XQPlots::setPickerCallback(OnPickerCallback cb) {
	xmprintf(8, "XQPlots::setPickerCallback!\n ");
	onPickerCallback = cb;
}

Q_INVOKABLE void XQPlots::drawAllMarkers1(int index, double x, double y, double z, double t) {
	drawAllMarkers(t);
		
	CBPickerInfo cbi;
	cbi.index = index;
	cbi.label[0] = 0;
	cbi.lineID = 0;
	cbi.plotID = 0;
	cbi.time = t;
	cbi.type = 2;
	cbi.x = x;
	cbi.y = y;
	cbi.xx = 0;
	cbi.yy = 0;
	cbi.z = z;

	if (onPickerCallback != 0) {
		//xmprintf(8, "XQPlots::drawAllMarkers2: onPickerCallback, time = %f \n", cbi.time);
		
		//  all this is called from inside QT GUI thread
		onPickerCallback(cbi);
	}

#ifdef ENABLE_UDP_SYNC
	sendPickerInfo(cbi);		//  send UDP info
	if (broadCastInfo != 0) {  //  send one more UDP?
		sendBroadcast(x, y, z);
	}
#endif
}

void XQPlots::pFilterThreadF() {
	using namespace std::chrono_literals;
	CBPickerInfo cbiLocal;

	while (!pleaseStopFilterThread) {
		pFilterMutex.lock();
		if (haveNewPickerInfo) {
			haveNewPickerInfo = false;
			memcpy(&cbiLocal, &cbi, sizeof(cbi));
			pFilterMutex.unlock();

			//drawAllMarkers(cbiLocal.time);
			QMetaObject::invokeMethod(this, "drawAllMarkers", Qt::QueuedConnection, Q_ARG(double, cbiLocal.time));

			if (onPickerCallback != 0) {  //  call the callback function?
				onPickerCallback(cbiLocal);
			}
#ifdef ENABLE_UDP_SYNC
			sendPickerInfo(cbi);  // send out UDP about the callback

			if (broadCastInfo != 0) {  //  send one more UDP?
				mxat(broadCastInfo->size > 0);
				long long i = findClosestPoint_1(0, broadCastInfo->size - 1, broadCastInfo->time, cbiLocal.time);
				broadCastInfo->ma.index = i;
				if ((i >= 0) && (i < broadCastInfo->size)) {
					sendBroadcast(broadCastInfo->x[i], broadCastInfo->y[i], broadCastInfo->z[i]);
				}
			}
#endif
		}	else {
			pFilterMutex.unlock();
		}

		std::this_thread::sleep_for(100ms);
	}

}

Q_INVOKABLE void XQPlots::drawAllMarkers2(int figureID, int lineID, int index, int fx, int fy, double x, double y, double t, const std::string& legend) {
	//xmprintf(8, "drawAllMarkers2!\n ");

		CBPickerInfo cbi1;
		cbi1.index = index;
		strncpy(cbi1.label, legend.c_str(), cbi1.lSize);
		cbi1.lineID = lineID;
		cbi1.plotID = figureID;
		cbi1.time = t;
		cbi1.type = 1;
		cbi1.x = x;
		cbi1.y = y;
		cbi1.xx = fx;
		cbi1.yy = fy;
		cbi1.z = 0.0;

		pFilterMutex.lock();
		haveNewPickerInfo = true;
		memcpy(&cbi, &cbi1, sizeof(cbi));
		pFilterMutex.unlock();
}

Q_INVOKABLE void XQPlots::drawAllMarkers(double t) {
	//if (!markersAreVisible) markersAreVisible = true;
    std::map<std::string, JustAplot*>::iterator it;
    for(it = figures.begin(); it != figures.end(); it++) {
	   it->second->drawMarker(t);
    }
	for (it = figures.begin(); it != figures.end(); it++) {
		it->second->replot();
	}
}
#ifdef ENABLE_UDP_SYNC
void XQPlots::sendBroadcast(double x, double y, double z) {
	if (bc == 0) {
		return;
	}
	BroadcastMessage m;
	memset(m.head, 'E', 4);
	memset(m.tail, 'F', 4);
	m.pos[0] = x;
	m.pos[1] = y;
	m.pos[2] = z;

	bc->bcSend((unsigned char*)(&m), sizeof(BroadcastMessage));


}
void XQPlots::sendPickerInfo(const CBPickerInfo& cpi) {
	if (bc == 0) {
		return;
	}
	PickerMessage m;
	memset(m.head, 'P', 4);
	memset(m.tail, 'T', 4);
	memcpy(&m.cpi, &cpi, sizeof(cpi));

	bc->bcSend((unsigned char*)(&m), sizeof(m));
}
void XQPlots::sendClipInfo(double t1, double t2, int clipGroup) {
	if (bc == 0) {
		return;
	}
	ClipUdpMessage m;
	memset(m.head, 'C', 4);
	memset(m.tail, 'L', 4);
	m.t1 = t1;
	m.t2 = t2;
	m.clipGroup = clipGroup;
	for (int i = 0; i < 3; i++) {
		m.pos1[i] = 0;
		m.pos2[i] = 0;
	}
	m.havePos = 0;
	if ((broadCastInfo != 0) && (broadCastInfo->size > 0)) {  
		long long i1 = findClosestPoint_1(0, broadCastInfo->size - 1, broadCastInfo->time, t1);
		long long i2 = findClosestPoint_1(0, broadCastInfo->size - 1, broadCastInfo->time, t2);
		if ((i1 >= 0) && (i1 < broadCastInfo->size) && (i2 >= 0) && (i2 < broadCastInfo->size)) {
			m.pos1[0] = broadCastInfo->x[i1];
			m.pos1[1] = broadCastInfo->y[i1];
			m.pos1[2] = broadCastInfo->z[i1];

			m.pos2[0] = broadCastInfo->x[i2];
			m.pos2[1] = broadCastInfo->y[i2];
			m.pos2[2] = broadCastInfo->z[i2];
			m.havePos = 1;
		}
	}

	bc->bcSend((unsigned char*)(&m), sizeof(m));
}
#endif

void XQPlots::clipAll(double t1, double t2, int clipGroup) {
	std::map<std::string, JustAplot*>::iterator it;
	for (it = figures.begin(); it != figures.end(); it++) {
		JustAplot* jsp = it->second;
		if (jsp->clipGroup == clipGroup) {
			jsp->onClip(t1, t2);
		}
	}
#ifdef ENABLE_UDP_SYNC
	sendClipInfo(t1, t2, clipGroup);
#endif
}

JustAplot* XQPlots::figure(int n, JPType type, unsigned int flags) {
	char buf[32];
	std::string fName;
	if (n == 0) { //  this means 'create new'
		for (int i = 500; i < 2500; i++) {
			snprintf(buf, 32, "%d", i); buf[31] = 0;
			fName.assign(buf);
			if (figures.find(fName) == figures.end()) { //   new value
				n = i;
				break;
			}
		}
	}
	
	snprintf(buf, 32, "%d", n); buf[31] = 0;
	fName.assign(buf);
	return figure(fName, type, flags);
}

void XQPlots::title(const std::string& s) {
	if (cf == 0) {
		return;
	}
	cf->title(s);

	//  change title in the tv:
	QList<QStandardItem *> si = pim.findItems(cf->key.c_str());
	if (si.size() > 0) {
		si.at(0)->setText(s.c_str());
	}
}
void XQPlots::footer(const std::string& s) {
	if (cf == 0) {
		return;
	}
	cf->footer(s);
}

/*
void XQPlots::setaxesequal() {
	if (cf == 0) {
		return;
	}
	//cf->setAxesEqual();
}
*/
void XQPlots::xlabel(const std::string& s) {
	if (cf == 0) {
		return;
	}
	cf->xlabel(s);
}

int XQPlots::service(int x) {
	switch (x) {
		case qsCallbackStarted: 
			callbackWorking = true;
		break;
		case qsCallbackFinished:
			callbackWorking = false;
		break;
	};
	return 0;
}

void XQPlots::ylabel(const std::string& s) {
	if (cf == 0) {
		return;
	}
	cf->ylabel(s);
}

void  XQPlots::setClipGroup(int cg) {
	currentClipGroup = cg;
}

int  XQPlots::plot(double* x, double* y, int size, const char* name, 
					     const char* style, int lineWidth, int symSize,
						double* time) {
	mxassert((x != 0) && (y != 0) && (size > 0) && (name != 0) && (style != 0), "");
	if (cf == 0) {
		figure(0, jQWT);
	} else {
		if (cf->type == jMathGL) { // create another one here
			figure(0, jQWT);
		}
	}

	linesHistoryCounter += 1;

	//it will be deleted in 'cf' destructor
	LineItemInfo* i = new LineItemInfo(x, y, size, name, (time == 0) ? 2 : 3, time);
	if (!i->ok) {
		xmprintf(0, "ERROR in XQPlots::plot: cannot create line (%s)(%s) \n", name, style);
		delete i;
		return -2;
	}
	i->style = style; 
	i->lineWidth = lineWidth;
	i->symSize = symSize;
	i->important = currentImportanceMode;
	i->id = linesHistoryCounter;

	cf->addLine(i);

	
	lines[linesHistoryCounter] = LineHandler{i, cf};
	xmprintf(5, "XQPlots::plot: line [%s](%d) added\n", i->legend.c_str(), linesHistoryCounter);

	return linesHistoryCounter;
}

void XQPlots::mesh(const MeshInfo& info) {
	xmprintf(6, "\tXQPlots::mesh starting \n");
	if (cf == 0) {
		figure(0, jMathGL);
	} else {
		if (cf->type != jMathGL) {
			figure(0, jMathGL);
		}
	}
	if (cf->type != jMathGL) {
		return;
	}
	xmprintf(6, "\tXQPlots::mesh adding.... \n");
	cf->addMesh(info);
	xmprintf(6, "\tXQPlots::mesh finished \n");
}

int  XQPlots::plot(double* x, double* y, double* z, int size, const char* name,
	const char* style, int lineWidth, int symSize,
	double* time) {
	mxassert((x != 0) && (y != 0) && (z != 0) && (size > 0) && (name != 0) && (style != 0), "");
	xmprintf(5, "\tXQPlots::plot  start \n");
	if (cf == 0) {
		xmprintf(5, "\t\tXQPlots::plot  creating another 'figure' (1) \n");
		figure(0, jMathGL);
		xmprintf(5, "\t\t - created \n");
		
	} else { ///  check:
		if (cf->type != jMathGL) {
			xmprintf(5, "\t\tXQPlots::plot  creating another 'figure' (3) \n");
			figure(0, jMathGL);
			xmprintf(5, "\t\t - created \n");
		} else {
			xmprintf(5, "\tXQPlots::plot  have a good window already \n");
		}
	}
	if (cf->type != jMathGL) {
		xmprintf(0, "\t\tXQPlots::plot  cannot create  jMathGL object\n");
		return -1;
	}

	//it will be deleted in 'cf' destructor
	//xmprintf(5, "\t creating LineItemInfo.. \n");
	LineItemInfo* i = new LineItemInfo(x, y, z, size, name, time);
	//xmprintf(5, "\t\t created.\n");
	i->style = style;
	i->lineWidth = lineWidth;
	i->symSize = symSize;
	i->important = currentImportanceMode;

	xmprintf(5, "\tXQPlots::plot  adding line \n");
	cf->addLine(i);

	linesHistoryCounter += 1;
	lines[linesHistoryCounter] = LineHandler{ i, cf };
	xmprintf(5, "\tXQPlots::plot  finished \n");
	return linesHistoryCounter;
}

int XQPlots::removeLine(int key) {
	auto i = lines.find(key);
	if (i == lines.end()) {
		return 1;
	}
	LineHandler& h = i->second;
	h.plot->removeLine(h.line);
	try {
		lines.erase(key);
	} catch (const std::exception& ex) {
		xmprintf(1, "lines.erase(key) failed (%s)\n", ex.what());
	}

	return 0;
}
int XQPlots::changeLine(int key, double* x, double* y, double* z, double* time, int size) {
	auto i = lines.find(key);
	if (i == lines.end()) {
		return 1;
	}
	LineHandler& h = i->second;
	h.plot->changeLine(h.line, x, y, z, time, size);

	return 0;
}

#ifdef ENABLE_UDP_SYNC
void XQPlots::enableCoordBroadcast(double* x, double* y, double* z, double* time, int size) {
	if (broadCastInfo != 0) {
		delete broadCastInfo;
		broadCastInfo = 0;
	}
	broadCastInfo = new LineItemInfo(x, y, z, size, "broadcast", time);
	if (bc == 0) {
		bc = new BCUdpClient();
	}

	if (bServer == 0) {
		try {
			bServer = new BCUdpServer(qwSettings.udp_server_port);
		}	catch (const std::exception& ex) {
			xmprintf(0, "ERROR: XQPlots::enableCoordBroadcast: cannot create UDP server on address %d; check if this port in available (%s)\n",
				qwSettings.udp_server_port, ex.what());
			return;
		}
		xmprintf(2, "BCUdpServer probably started with port number %d \n", qwSettings.udp_server_port);
		bServer->bStart([&](double p[3]) { on3DMarker(p); });
		//(&XQPlots::on3DMarker);
	}

}
void XQPlots::disableCoordBroacast() {
	if ((broadCastInfo == 0) || (bc == 0)) {
		return;
	}
	char m[4]; memset(m, 'X', 4);
	bc->bcSend((unsigned char*)(&m), 4);
	delete broadCastInfo;
	broadCastInfo = 0;
}

#endif

void XQPlots::onExit() {
	//  stop picker info filter thread
	//  takes about 0.1 seconds
	pleaseStopFilterThread = true;
	if (pFilterThread.joinable()) {
		pFilterThread.join();
	}

	clearFigures();
	
#ifdef ENABLE_UDP_SYNC
	if (broadCastInfo != 0) {
		delete broadCastInfo;
		broadCastInfo = 0;
	}
	if (bc != 0) {
		delete bc;
		bc = 0;
	}
	if (bServer != 0) {
		delete bServer;  //  this should stop the server thread
		bServer = 0;
	}
#endif
}

void XQPlots::clearFigures() {
	clearingAllFigures = true;
	std::map<std::string,  JustAplot*>::iterator it = figures.begin();
	while (it != figures.end()) {
		cf = it->second;
		cf->close();
		delete cf; 
		cf = 0;
		it++;
	}
	figures.clear();
	lines.clear();
	clearingAllFigures = false;

	pim.clear();
}

void XQPlots::onSelection(const std::string& key) {
	if (callbackWorking) {
		return;
	}
	
	std::map<std::string,  JustAplot*>::iterator it = figures.find(key);
	if (it != figures.end()) {
		cf = it->second;
		emit selection(key);
	} else {  //  error?

	}
}

void XQPlots::onPicker(const std::string& key_, double X, double Y) {
	emit picker(key_, X, Y);
}

void XQPlots::onFigureClosed(const std::string& key) {
	if (clearingAllFigures) {
		return;
	}
	JustAplot* f = 0;
	std::map<std::string,  JustAplot*>::iterator it = figures.find(key);
	if (it != figures.end()) {
		f = it->second;  //  DO WE NEED THIS???
		
		//    remove all the lines from 'lines':
		for (auto i = lines.begin(); i != lines.end(); ) { //  according to https://en.cppreference.com/w/cpp/container/map/erase
			if (i->second.plot == f) {
				i = lines.erase(i);
			} else {
				++i;
			}
		}

		//  remove from tv:
		QList<QStandardItem *> si = pim.findItems(f->name.c_str());
		if (si.size() != 0) {
			QModelIndex mi = si.at(0)->index();
			pim.removeRow(mi.row());
		}

		emit figureClosed(key);
		
		// !!!!!!!!!!!!!!delete cf;    because of setAttribute(Qt::WA_DeleteOnClose);
		disconnect(f, 0, 0, 0);
		figures.erase(it);

	}   else  {
		//   error? 
		xmprintf(4, "ERROR: XQPlots::onFigureClosed for key = {%s} \n", key.c_str());
	}

	if (f == cf) { // we need new 'cf'
		//   set cf   to the new value:
		it = figures.begin();
		if (it != figures.end()) {
			cf = it->second;
		} else { 
			cf = 0;
		}
	}
}

//void XQPlots::setEcefOrigin(double* origin) {
//	memcpy(ecefOrigin, origin, 3*sizeof(double));
//}

