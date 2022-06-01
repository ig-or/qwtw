


#include "xstdef.h"
#include "qwproc.h"

#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef WIN32	
	#include "windows.h"
	#include <shlobj.h>
	#include <psapi.h>
#else
	#include <syslog.h>
	#include <unistd.h>
	#include <pwd.h>
#endif

#include <string>
#include <iostream>
#include <sstream>

#include "boost/filesystem.hpp"

#ifdef WIN32
//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
std::string GetLastErrorAsString(DWORD errorMessageID) {
	if (errorMessageID == 0) {
		return std::string(); //No error message has been recorded
	}

	LPSTR messageBuffer = nullptr;

	//Ask Win32 to give us the string version of that message ID.
	//The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	//Copy the error message into a std::string.
	std::string message(messageBuffer, size);

	//Free the Win32's string's buffer.
	LocalFree(messageBuffer);

	return message;
}
#endif


bool getFolderLocation(char* p, int pSize) {
	using namespace boost::filesystem;
	boost::system::error_code ec;
	if (pSize < 2) {
		return false;
	}
	p[0] = 0;

	char hPath[512];
	char* ePath = getenv("HOME");
	if (ePath == nullptr) {  // "HOME" may be empty 
		ePath = getenv("USERPROFILE");    // " this can work on Windows
		if (ePath == nullptr) {
#ifdef WIN32
			HRESULT result = SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, hPath); // one more test for Windows
			if (SUCCEEDED(result)) {
				ePath = hPath;
			} else {
				xmprintf(0, "ERROR: cannot find a path to the config file\n");
				return false;
			}
#else	// other options for Linux? 
			struct passwd* pw = getpwuid(getuid());
			if (pw == NULL) {
				xmprintf(0, "ERROR: cannot find a path to the config file\n");
				return false;
			}
			ePath = pw->pw_dir;
#endif
		}
	}
#ifdef DEBUG123
	//strncpy(p, dir.string().c_str(), pSize);
	#ifdef WIN32
		snprintf(p, pSize, "%s\\.qwtw", ePath);
	#else
		snprintf(p, pSize, "%s/.qwtw", ePath);
	#endif
#else
	path home(ePath);
	path dir = home / ".qwtw";
	strncpy(p, dir.string().c_str(), pSize);
#endif
	return true;
}

int checkProcRunning() {
	using namespace boost::filesystem;
	boost::system::error_code ec;
	char p[512];
	xmprintf(2, "\n======checkProcRunning()============\n");
	if (getFolderLocation(p, 512)) {

	} else {
		xmprintf(0, "\tERROR: cannot obtain lock file location\n");
		return 5;
	}
	xmprintf(2, "getFolderLocation: %s\n", p);
	path dir = path(p) / "lock";
	path f = dir / "qwproc";
	xmprintf(2, "\tcheckProcRunning() starting\n");
	try {
		xmprintf(3, "\tfile f: %s, %s, %s \n", f.string().c_str(), absolute(f).string().c_str(), canonical(f).string().c_str());
	} catch  (std::exception& ex) {
		xmprintf(3, "exception %s\n", ex.what());
	}
	if (exists(f, ec)) {  //   check the lock
		xmprintf(2, "\tfile %s exists; ec = %s\n", f.string().c_str(), ec.message().c_str());
		int fs = file_size(f);
		std::string sp;
		std::ifstream lockFile(f.string());
		std::getline(lockFile, sp);
		lockFile.close();
		xmprintf(2, "\tcheckProcRunning: pid from file %s: %s\n",f.string().c_str(),  sp.c_str());

		//  do we still have this process?
#ifdef WIN32
		DWORD pid = strtoul(sp.c_str(), 0, 10);
		xmprintf(2, "\tpid = %u\n", pid);
		HANDLE ph = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
		if (ph != NULL) {   //  ok, we have something
			DWORD status = STILL_ACTIVE;
			BOOL test = GetExitCodeProcess(ph, &status);
#ifndef qwtwcEXPORTS
			xmprintf(2, "\tgot the handle.. GetExitCodeProcess() returned %d, status = %u \n", test, status);
#endif
			if (test) { // ....
				if (status == STILL_ACTIVE) {
					xmprintf(3, "\tsomething is running. GetExitCodeProcess for %d returned 'STILL_ACTIVE'\n", pid);

					//  lets check what we have here:
					char fName[512];
					DWORD test14 = GetModuleFileNameExA(ph, NULL, fName, 512);
					if (test14 == 0) { //  this not working
						DWORD error14 = GetLastError();
						std::string s14 = GetLastErrorAsString(error14);
						xmprintf(2, "GetModuleFileNameExA failed; error = %d; (%s)  \n", error14, s14.c_str());

						return 0; // lets return 'not running'
					} else {
						fName[511] = 0;
						xmprintf(3, "we got following with this PID: %s\n", fName);
						if (strstr(fName, "qwproc") == NULL) {
							xmprintf(3, "\tthis was not our plot server \n");
							return 0;
						} else {
							xmprintf(3, "\tthis looks like our plot server \n");
							return 1;
						}

					}


					return 1; //  running!
				} else {
					xmprintf(2, "\tnot running!\n");
				}
			}	else {
				LPVOID lpMsgBuf;
				//LPVOID lpDisplayBuf;
				DWORD dw = GetLastError();
				if (dw == 5) {  //  looks like running
					return 1; 
				}

				FormatMessage(
					FORMAT_MESSAGE_ALLOCATE_BUFFER |
					FORMAT_MESSAGE_FROM_SYSTEM |
					FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					dw,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPTSTR)&lpMsgBuf,
					0, NULL);

				xmprintf(1, "\tGetExitCodeProcess returned error # %d (%s)\n", dw, (LPTSTR)lpMsgBuf);

			}
		}	else { //    looks like not running
			xmprintf(3, "\tOpenProcess returned NULL\n");
		}
#else
		path proc = path("/proc") / sp / "status";
		if (exists(proc)) { // lets check the name
			//path pf = proc / "status";
			std::string pName;
			std::ifstream pFile(proc.string());
			std::getline(pFile, pName);
			xmprintf(2, "lockHandle: %s is running\n", pName.c_str());
			if (pName.find("qwproc") == std::string::npos) {
				xmprintf(2, "\tbut looks like this is different program with same pid\n");
			} else { // its me ?
				//  lets check the status:
				std::string line;
				for (line; std::getline(pFile, line); ) {
					if (line.find("State:") != std::string::npos) {
						if (line.find('Z')  != std::string::npos) {
							xmprintf(3, "\tprocess (%s) zombie? (%s)\n", sp.c_str(), line.c_str());
						} else {
							xmprintf(2, "\tit's me; [%s]\n", line.c_str());
							pFile.close();
							return 1;
						}
						break;
					}
				}

			}
			pFile.close();
		} else {
			xmprintf(2, "checkProcRunning: no %s file detected \n", proc.string().c_str());
		}
#endif
	} else {
		xmprintf(2, "\tcheckProcRunning(): no lock file detected\n");
	}
	return 0; //  not running
}

CmdSync::CmdSync() :	cbInfoMutex(boost::interprocess::open_or_create,  "qwtw_cbInfoMutex"),
						cbWait(boost::interprocess::open_or_create, "qwtw_cbWait")  {

}
CmdSync::~CmdSync() {
	cbInfoMutex.remove("qwtw_cbInfoMutex");
	cbWait.remove("qwtw_cbWait");
}

