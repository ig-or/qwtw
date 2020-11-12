


#include "xstdef.h"
#include "qwproc.h"

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>

#include <string>
#include <iostream>
#include <sstream>

#include "boost/filesystem.hpp"

int checkProcRunning() {
	using namespace boost::filesystem;
	boost::system::error_code ec;
	path home(getenv("HOME"));
	path dir = home / ".qwtw" / "lock";
	path f = dir / "qwproc";
	//printf("file f: %s, %s, %s \n", f.string().c_str(), absolute(f).string().c_str(), canonical(f).string().c_str());
	if (exists(f, ec)) {  //   check the lock
		//printf("file %s exists; ec = %s\n", f.string().c_str(), ec.message().c_str());
		int fs = file_size(f);
		std::string sp;
		std::ifstream lockFile(f.string());
		std::getline(lockFile, sp);
		lockFile.close();
		//printf("lockHandle: pid from file %s: %s\n",f.string().c_str(),  sp.c_str());

		//  do we still have this process?
		path proc = path("/proc") / sp / "status";
		if (exists(proc)) { // lets check the name
			//path pf = proc / "status";
			std::string pName;
			std::ifstream pFile(proc.string());
			std::getline(pFile, pName);
			pFile.close();
			//printf("lockHandle: %s is running\n", pName.c_str());
			if (pName.find("qwproc") == std::string::npos) {
				printf("\tbut looks like this is different program with same pid\n");
			} else { // its me
				//printf("\tit's me\n");
				return 1;
			}
		} else {
			//printf("lockHandle: no %s file detected \n", proc.string().c_str());
		}
	} else {
		//printf("no lock file detected\n");
	}
	return 0; 
}

