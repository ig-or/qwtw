

#ifndef LOCKING_QUEUE_H_FILE
#define LOCKING_QUEUE_H_FILE

#include <mutex>
#include <condition_variable>
//#include "xmroundbuf.h"
#include <boost/circular_buffer.hpp>
//#include <boost/interprocess/sync/interprocess_semaphore.hpp>

// http://stackoverflow.com/questions/17853875/concurrent-blocking-queue-in-c11
struct operation_aborted {};

template <class T, std::size_t N>
class QueueBlock {
public:
    typedef T value_type;
    QueueBlock(): q_(N), aborted_(false), wrs(0) {   }
    void push(value_type data, bool waitForReply = false) {
	   std::unique_lock<std::mutex> lk(mtx_);
	   cv_pop_.wait(lk, [=] { return !q_.full() || aborted_; });
	   if(aborted_) throw operation_aborted();
	   q_.push_back(data);
	   cv_push_.notify_one();

	   if(waitForReply) {
		   wrs.wait();
	   }
    }
    void sendReply() {
	    wrs.post();
    }

    value_type pop() {
	   std::unique_lock<std::mutex> lk(mtx_);
	   cv_push_.wait(lk, [=] { return !q_.empty() || aborted_; });
	   if(aborted_) throw operation_aborted();
	   value_type result = q_.front();
	   q_.pop_front();
	   cv_pop_.notify_one();
	   return result;
    }
    void abort() {
	   std::lock_guard<std::mutex> lk(mtx_);
	   aborted_ = true;
	   cv_pop_.notify_all();
	   cv_push_.notify_all();
    }
private:
    boost::circular_buffer<value_type> q_;
    bool aborted_;
    std::mutex mtx_;
    std::condition_variable cv_push_;
    std::condition_variable cv_pop_;

   // boost::interprocess::interprocess_semaphore    wrs;
};



#endif