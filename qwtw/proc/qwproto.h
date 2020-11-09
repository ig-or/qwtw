

#include "qwproc.h"
////#include <boost/interprocess/shared_memory_object.hpp>
//#include <boost/interprocess/mapped_region.hpp>
#include <thread>

namespace boost { namespace interprocess {
class shared_memory_object;
class mapped_region;
}}

class QProcInterface {

public:
    QProcInterface();
    ~QProcInterface();
    void start();
    void stop();

private:
    ProcData pd;
    boost::interprocess::shared_memory_object* shmCommand;
    boost::interprocess::shared_memory_object* shmDataX;
    boost::interprocess::shared_memory_object* shmDataY;
    boost::interprocess::shared_memory_object* shmDataT;

    boost::interprocess::mapped_region* commandReg;
    bool started;
    std::thread  wThread;


    void run();
};
