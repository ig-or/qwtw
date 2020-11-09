#include "qwproc.h"

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

#define xm_printf printf

int main(int argc, char** argv) {
    setbuf(stdout, NULL);
	setvbuf(stdout, NULL, _IONBF, 0);
   	using namespace boost::interprocess;

    printf("TEST main: starting \n");
    shared_memory_object shmCommand(open_only, "QWTWCommand", read_write);
    shmCommand.truncate(sizeof(CmdHeader));
    mapped_region commandReg = mapped_region(shmCommand, read_write);

    ProcData pd;
    //pd.hdr = (CmdHeader*)commandReg.get_address();
    //pd.hdr = new (commandReg->get_address()) CmdHeader;
    pd.hdr = static_cast<CmdHeader*>(commandReg.get_address());

    scoped_lock<interprocess_mutex> lock(pd.hdr->mutex);
    printf("cmd = %d  \n", pd.hdr->cmd);
    pd.hdr->cmd = CmdHeader::exit;
    pd.hdr->cmdWait.notify_all();
    printf("TEST: start waiting ..\n");
    pd.hdr->workDone.wait(lock);

    
    
    printf("TEST main: exiting \n");
    return 0;
}


void assert_failed(const char* file, unsigned int line, const char* str) {
	xm_printf("ASSERT faild: %s (file %s, line %d)\n", str, file, line);
}


