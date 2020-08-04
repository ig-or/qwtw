/**
	A QWidget, which will handle incoming commands.
*/

#pragma once

#include <QWidget>
#include <QThread>
//#include <boost/lockfree/queue.hpp>
//#include <boost/thread/thread.hpp>
//#include "queue_block.h"

//#include "lrbuf.h"

/** 'message to QT' item
*/
struct BQInfo {
	static const int dsize = 128;
	int cmd;
	char data[dsize];
};
Q_DECLARE_METATYPE(BQInfo);

//class QTS2;

/** internal class, which will catch messages and send them into QTSMainWidget
*/
class QTS2: public QThread {
    Q_OBJECT
public:
    QTS2(LRBuf<BQInfo, 16>* bq_);

signals:
    void gotNewInfo(BQInfo x);

protected:
    virtual void run();

private:
	LRBuf<BQInfo, 16>* bq;
    bool exitFlag;
    BQInfo bqInfo;
};



/** This is our 'main' widget. Create a subclass of
 * this, and reimplement 'onInfo()'

*/
class QTSMainWidget: public QWidget  {
	Q_OBJECT
public:
	QTSMainWidget(LRBuf<BQInfo, 16>* bq_);
	virtual ~QTSMainWidget();

    /** send a command into QT */
	void command(BQInfo x);
    void start();

protected:
	virtual void onInfo(BQInfo x) = 0;
	void stopQueueThread();
	/** send acknowladge back to the host

	*/
	void sendReply() {  
		if(bq != 0) {
			bq->ack();
		}
	}

private slots:
	void qappQuit();
	void onInfoSlot(BQInfo x);
	
private:
	LRBuf<BQInfo, 16>* bq;
  //  boost::thread* bt;
	QTS2* qts2;
	
};




