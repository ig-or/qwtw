

#include "qwproc.h"
////#include <boost/interprocess/shared_memory_object.hpp>
//#include <boost/interprocess/mapped_region.hpp>
#include <thread>

namespace boost { namespace interprocess {
class shared_memory_object;
class mapped_region;
}}

class QWorker;
class QApplication;

class QProcInterface {

public:
    QProcInterface(QWorker& worker_, QApplication& app_);
    ~QProcInterface();

    ///  start everything
    void start();
    

private:
    ProcData pd;
    boost::interprocess::shared_memory_object* shmCommand;
    boost::interprocess::shared_memory_object* shmDataX;
    boost::interprocess::shared_memory_object* shmDataY;
    boost::interprocess::shared_memory_object* shmDataT;

    boost::interprocess::mapped_region* commandReg;
    bool started;
    bool needStopThread;
    std::thread  wThread;
    
    QWorker& worker;
    QApplication& app;

    /// stop the thread
    void stop();
    void run();
    void processCommand(int cmd);
};
