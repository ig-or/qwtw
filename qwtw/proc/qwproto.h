

#include "qwproc.h"
////#include <boost/interprocess/shared_memory_object.hpp>
//#include <boost/interprocess/mapped_region.hpp>

// therer are problems on mingw with this #include <thread>
#include <memory>
#include <boost/thread/thread.hpp>
#include <thread>
#include <mutex>
#include <chrono>
#include <thread>
#include <condition_variable>

namespace boost { 
    namespace interprocess {
        class shared_memory_object;
        class mapped_region;
    }
    //class thread;

}

class QWorker;
class QApplication;

class QProcInterface {

public:
/**
 *  \param mdPath path to the MArble data files
 */
    QProcInterface(QWorker& worker_, QApplication& app_);
    ~QProcInterface();

    ///  start everything
    void start();

    /**
     * \return true if already running
    **/
    static bool runningAlready();

    void onCB(const CBPickerInfo& cbi_);
    

private:
    ProcData pd;
    boost::interprocess::shared_memory_object* shmCommand;
    boost::interprocess::shared_memory_object* shmDataX;
    boost::interprocess::shared_memory_object* shmDataY;
    boost::interprocess::shared_memory_object* shmDataZ;
    boost::interprocess::shared_memory_object* shmDataT;
    boost::interprocess::shared_memory_object* shmDataData;

    boost::interprocess::mapped_region* commandReg;
    boost::interprocess::mapped_region* xDataReg;
    boost::interprocess::mapped_region* yDataReg;
    boost::interprocess::mapped_region* zDataReg;
    boost::interprocess::mapped_region* tDataReg;
    boost::interprocess::mapped_region* dataDataReg;
    bool started;
    bool needStopThread;
    //std::thread  wThread;
    std::shared_ptr<boost::thread> wThread;
    std::thread cbFilterThread;

    QWorker& worker;
    QApplication& app;

    /// stop the thread
    void stop();
    void run();
    void cbFilterThreadF();
    std::mutex cbFilterMutex;
    CBPickerInfo cbi; 
    //std::chrono::time_point< std::chrono::system_clock > cbiTime;
    bool haveNewCallbackInfo = false;
    //std::condition_variable cbiReady;


    void processCommand(int cmd);
    //void setupSHM1(long long size, boost::interprocess::shared_memory_object* shm, boost::interprocess::mapped_region* reg);

    //  change size of the memory segments
    void changeSize(long long newSize);
    void changeDataSize(long long newSize);
    void removeSHM();
    void plot();
};
