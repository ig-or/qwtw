

#include "xstdef.h"
#include "xmutils.h"

#include <QCloseEvent>
#include "qtswidget.h"

#include "xqbytebuffer.h"
#include "xstdef.h"
#include "xmutils.h"
#include "lrbuf.h"

#include <boost/array.hpp>
#include <boost/chrono/thread_clock.hpp>


QTS2::QTS2(LRBuf<BQInfo, 16>* bq_) : exitFlag(false), bq(bq_) {
    //xm_printf("QTS2  created \n");
}

void QTS2::run() {
	while(!exitFlag) {
		try {
			bqInfo = bq->take();
		}	catch (...) {  // should we exit from this thread?
			bqInfo.cmd = 0;
			bqInfo.data[0] = 0;
            //xm_printf("QTS2::run(): got an exception inside queue::pop \n");
		}
        //xm_printf("QTS2::run(): got new message #%d\n", bqInfo.cmd);

		switch(bqInfo.cmd) {

		};
		emit gotNewInfo(bqInfo);
		//emit gotNewInfo(bqInfo.cmd);

		if ((bqInfo.cmd == 0)) {  //  exit right now!
            //xm_printf("QTS2: got exit signal\n");
			exitFlag = true;
			break;
		}
	}
    //xm_printf("exiting QTS2 thread \n");
}


QTSMainWidget::QTSMainWidget(LRBuf<BQInfo, 16>* bq_) : QWidget(0, 0), bq(bq_) {
	//connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(qappQuit()));
	qts2 = new QTS2(bq_);
	bool OK = false;
	OK = connect(qts2, SIGNAL(gotNewInfo(BQInfo)), this, SLOT(onInfoSlot(BQInfo)));
	//OK = connect(qts2, SIGNAL(gotNewInfo(int)), this, SLOT(onInfoSlot(int)));
	xm_printf("QTSMainWidget  created \n");

}

QTSMainWidget::~QTSMainWidget() {
	stopQueueThread();
}

/*
void QTSMainWidget::onInfoSlot(BQInfo x) {
    //xm_printf("QTSMainWidget::onInfo() [%u] \n", x.cmd);
	onInfo(x);
}
*/
void QTSMainWidget::onInfoSlot(BQInfo x) {
    //xm_printf("QTSMainWidget::onInfo() [%u] \n", x.cmd);
	onInfo(x);
}


void QTSMainWidget::qappQuit() {
    //xm_printf("QTSMainWidget::qappQuit() \n");
	stopQueueThread();
}

void QTSMainWidget::stopQueueThread() {
	if (qts2->isRunning()) {
        //xm_printf("QTSMainWidget::stopThread(): stopping the 'queue' thread....... \n");
		BQInfo stopInfo; stopInfo.cmd = 0;
		bq->put(stopInfo);
		bool ok = qts2->wait(80);
		if (!ok) { // wait timeout!
            //xm_printf("QTSMainWidget::stopThread(): wait timeout! \n");
			qts2->terminate();
        } else {
            //xm_printf("QTSMainWidget::stopThread(): OK\n");
        }
	}
}

void QTSMainWidget::command(BQInfo x) {
	bq->put(x);
}

void QTSMainWidget::start() {
	qts2->start();
}



