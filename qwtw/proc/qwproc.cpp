


#include "xstdef.h"
#include "qwproc.h"


#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef WIN32	
	#include "windows.h"
	#include "shlobj_core.h"
#else
	#include <syslog.h>
	#include <unistd.h>
#endif

#include <string>
#include <iostream>
#include <sstream>


#include "boost/filesystem.hpp"

bool getFolderLocation(char* p, int pSize) {
	using namespace boost::filesystem;
	boost::system::error_code ec;
	char hPath[512];
	char* ePath = getenv("HOME");
	if (ePath == nullptr) {
		ePath = getenv("USERPROFILE");
		if (ePath == nullptr) {
#ifdef WIN32
			HRESULT result = SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, hPath);
			if (SUCCEEDED(result)) {
				ePath = hPath;
			}
			else {
				xm_printf("ERROR: cannot find a path to the config file\n");
				return false;
			}
#else	
			xm_printf("ERROR: cannot find a path to the config file\n");
			return false;
#endif
		}
	}

	path home(ePath);
	path dir = home / ".qwtw";
	strncpy(p, dir.string().c_str(), pSize);
	return true;
}

int checkProcRunning() {
	using namespace boost::filesystem;
	boost::system::error_code ec;
	char p[512];
	if (getFolderLocation(p, 512)) {

	} else {
		xm_printf("ERROR: cannot obtain lock file location\n");
		return 5;
	}
	path dir = path(p) / "lock";
	path f = dir / "qwproc";
#ifndef qwtwcEXPORTS
	xm_printf("checkProcRunning() starting\n");
	//xm_printf("file f: %s, %s, %s \n", f.string().c_str(), absolute(f).string().c_str(), canonical(f).string().c_str());
#endif
	if (exists(f, ec)) {  //   check the lock
	#ifndef qwtwcEXPORTS
		xm_printf("file %s exists; ec = %s\n", f.string().c_str(), ec.message().c_str());
	#endif
		int fs = file_size(f);
		std::string sp;
		std::ifstream lockFile(f.string());
		std::getline(lockFile, sp);
		lockFile.close();
#ifndef qwtwcEXPORTS
		xm_printf("checkProcRunning: pid from file %s: %s\n",f.string().c_str(),  sp.c_str());
#endif

		//  do we still have this process?
#ifdef WIN32
		DWORD pid = strtoul(sp.c_str(), 0, 10);
		HANDLE ph = OpenProcess(SYNCHRONIZE, TRUE, pid);
		if (ph != NULL) {   //  ok, we have something
			DWORD status;
			BOOL test = GetExitCodeProcess(ph, &status);
			if (test) { // ....
				if (status == STILL_ACTIVE) {
					return 1; //  running!
				}
			}
		}
#else
		path proc = path("/proc") / sp / "status";
		if (exists(proc)) { // lets check the name
			//path pf = proc / "status";
			std::string pName;
			std::ifstream pFile(proc.string());
			std::getline(pFile, pName);
			pFile.close();
#ifndef qwtwcEXPORTS			
			xm_printf("lockHandle: %s is running\n", pName.c_str());
#endif			
			if (pName.find("qwproc") == std::string::npos) {
				#ifndef qwtwcEXPORTS
				xm_printf("\tbut looks like this is different program with same pid\n");
				#endif
			} else { // its me
#ifndef qwtwcEXPORTS
				xm_printf("\tit's me\n");
#endif
				return 1;
			}
		} else {
#ifndef qwtwcEXPORTS			
			xm_printf("checkProcRunning: no %s file detected \n", proc.string().c_str());
#endif			
		}
#endif
	} else {
#ifndef qwtwcEXPORTS		
		xm_printf("checkProcRunning(): no lock file detected\n");
#endif		
	}
	return 0; //  not running
}

