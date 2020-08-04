/**
 locked round buffer, using boost

*/

#ifndef LRBUF_H_FILE
#define LRBUF_H_FILE

#include "xmroundbuf.h"
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>

template <class T, int N> class LRBuf: public XMRoundBuf<T, N> {
public:
	LRBuf(): sem(0), ackSem(0) {

	}
	/**    put something
	*/
	void put(const T& x, bool waitForReply = false) {
		mutex.lock(); //   do almost everything inside the lock
		int n = XMRoundBuf<T, N>::num;
		XMRoundBuf<T, N>::put(x);
		if(XMRoundBuf<T, N>::num > n) {
			sem.post();
		}
		mutex.unlock();

		if(waitForReply) { // should we wait for reply?
			ackSem.wait();  //  perhapse
		}
	}

	/**   try to get something
	@return true if OK, false if failed
	*/
	bool get(T* x = NULL) {
		bool ret = false;
		if(sem.try_wait()) { //  looks like we have something....    let's check:
			mutex.lock();
			int n = XMRoundBuf<T, N>::num;
			if(ret = XMRoundBuf<T, N>::get(x)) { //   yes, we have indeed!

			}
			mutex.unlock();
		}
		return ret;
	}

	/**   wait until someboody would 'put'

	*/
	T take() {
		T ret;
		bool ok = false;
		sem.wait();  // wait for 'put'
		mutex.lock();
		ok = XMRoundBuf<T, N>::get(&ret);
		mxat(ok);
		mutex.unlock();
		return ret;
	}

	/** send an acknowladge; k

	*/
	void ack() {
		ackSem.post();
	}

private:
	boost::interprocess::interprocess_mutex mutex;
	boost::interprocess::interprocess_semaphore sem, ackSem;
};

#endif
