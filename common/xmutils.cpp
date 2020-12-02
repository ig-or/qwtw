
/**
Utilitis which do not belong to any calss.
\file    xmutils.cpp
\author   Igor Sandler

\version 1.0

*/



#include "xstdef.h"
//#include "xmatrixplatform.h"
#include "xmutils.h"
#include <string.h>

#ifdef LIN_UX
	#include <limits.h>
	#include <unistd.h>
#endif

#ifdef WIN32
#include <windows.h>
#include <stdlib.h>
#ifdef _DEBUG
	#include <stdio.h>
	#include <varargs.h>
	#include <tchar.h>
	#include <iostream>
#endif
#include <shlobj.h>

void w2str(wchar_t* src, char* dst) {
	int length = ::WideCharToMultiByte(CP_UTF8, 0, src, -1, NULL, 0, NULL, NULL);
	::WideCharToMultiByte(CP_UTF8, 0, src, -1, dst, length, NULL, NULL);
	dst[length] = 0;
}

void tchar2str(TCHAR* src, char* dst) {
#ifdef  UNICODE 
    w2str(src, dst);
#else
    strcpy(dst, src);
#endif
}
std::string tchar2stdstring(TCHAR* src) {
    std::string ret;
#ifdef  UNICODE 
    char* stmp = new char[_tcslen(src) * 3];
    w2str(src, stmp);
    ret = stmp; 
    delete[] stmp;
#else
    ret = src;
#endif
    
    return ret;
}

std::string getTemporalPath() {
    TCHAR lpTempPathBuffer[MAX_PATH];
    LPCTSTR errMsg;
    std::string ret("c:\\tmp");
    DWORD dwRetVal = 0;
    dwRetVal = GetTempPath(MAX_PATH,  lpTempPathBuffer); // buffer for path 
    if(dwRetVal > MAX_PATH || (dwRetVal == 0)) {
	   xm_printf("@WARNING: cannot get access to tmp folder\n");
	   return ret;
    }
    ret = tchar2stdstring(lpTempPathBuffer);
    return ret;
}

/*
CSIDL_COMMON_APPDATA  FOLDERID_ProgramData
The file system directory that contains application data for all users. 
A typical path is C:\Documents and Settings\All Users\Application Data. 
This folder is used for application data that is not user specific. 
For example, an application can store a spell-check dictionary, 
a database of clip art, or a log file in the CSIDL_COMMON_APPDATA folder. 
This information will not roam and is available to anyone using the computer.
*/

std::string getCommonAppDataPath() {  //CSIDL_COMMON_APPDATA
    std::string ret("c:\\atmp");
    TCHAR buf[MAX_PATH];
    BOOL ok = FALSE;
    ok = SHGetSpecialFolderPath(0, buf, CSIDL_COMMON_APPDATA, TRUE);
    if(!ok) {
	   xm_printf("@WARNING: cannot get access to COMMON_APPDATA folder\n");
	   return ret;
    }
    ret = tchar2stdstring(buf);
    return ret;
}

std::string getCurrentPath() {
	//    code from Miguel below:

	// Read current working directory
	TCHAR wd_buf[MAX_PATH];
	DWORD dwRet;
	dwRet = GetCurrentDirectory(MAX_PATH, wd_buf);
#ifdef _UNICODE
	std::wstring wide_wd(&wd_buf[0]);
	std::string working_dir(wide_wd.begin(), wide_wd.end());
	return working_dir;
#else
	std::string sbuf(&wd_buf[0]);
	return sbuf;
#endif
}

#endif

#ifdef LIN_UX
std::string getTemporalPath() { 
	std::string tp = ".";

	return tp;

}

std::string getCommonAppDataPath() {  

	std::string cadPath = ".";
	return cadPath;
}

#endif

#ifdef PC__VERSION
#ifdef XQBUILDSYSTEM
#include "build_info.h"
#include "build_number.h"
//int xqversion(char* vstr) {
//	int bs = sprintf(vstr, "\n(%s) v %s; bn #%s; compiled %s, platform %s",
//		XQX9STR(OURPROJECTNAME), VERSION, BUILD_NUMBER, COMPILE_TIME, OUR_PLATFORM);
//	return bs;
//}
int xqversion(char* vstr, int bufSize) {
	if (bufSize < 2) {
		return 0;
	}
#ifdef WITH_VERSION_INFO
	#if defined(_MSC_VER) && _MSC_VER < 1900
		int bs = snprintf(vstr, bufSize, "\n(%s) v %s; bn #%s; compiled %s, platform %s\n%s",
			XQX9STR(OURPROJECTNAME), VERSION, BUILD_NUMBER, COMPILE_TIME, OUR_PLATFORM,
			GIT_INFO);
	#else
		int bs = snprintf(vstr, bufSize, "\n(%s) v %s; bn #%s; compiled %s, platform %s\n%s",
			XQX9STR(OURPROJECTNAME), VERSION, BUILD_NUMBER, COMPILE_TIME, OUR_PLATFORM,
			GIT_INFO);
	#endif
#else // no version info
	int bs = snprintf(vstr, bufSize, "\nbuilt without version info support\n");
#endif
		vstr[bufSize - 1] = 0;
		vstr[bufSize - 2] = 0;
	return bs;
}

#ifdef WIN32
int xqversion(char* vstr, int bufSize, void* hModule) {
	int bs = xqversion(vstr, bufSize);

	char dllPath[MAX_PATH];
	DWORD dw = GetModuleFileNameA((HMODULE)(hModule), dllPath, MAX_PATH);

	dllPath[MAX_PATH-1] = 0; dllPath[MAX_PATH-2] = 0;

#if defined(_MSC_VER) && _MSC_VER < 1900
	int ret = sprintf_s(vstr + bs, bufSize - bs, " loaded from %s", dllPath);
#else
	int ret = snprintf(vstr + bs, bufSize - bs, " loaded from %s", dllPath);
#endif
	return ret;
}
#endif
#endif
#endif


bool rcvIsNan(float f) {
    unsigned int a = (unsigned int)f;
    if(a == 0x7FC00000) {
	   return true;
    } else {
	   return false;
    }

}

#if defined (LIN_UX) || defined (WIN32)
void getExeFilePath(char* path, int pathSize) {
	path[0] = 0;
    #ifdef LIN_UX
         int len = readlink("/proc/self/exe", path, pathSize);
		 path[len] = 0;
		 /*
		 char* p = strrchr(path, '/');
		 if(p)  { 
			 *(p + 1) = 0;
		 } 	 else {
			 path[0] = 0;
		 }
		 */
	#elif defined(WIN32)
#if 1
		//TCHAR pa[_MAX_PATH];
		char pa[_MAX_PATH];
		DWORD bs = GetModuleFileNameA(NULL, pa, _MAX_PATH);
		if (bs > pathSize) bs = pathSize;
		strncpy(path, pa, pathSize - 1);
		//char* pValue = new char[256];
#endif
#if 0
		char* pValue;
		if (_get_pgmptr(&(pValue)) == 0) {
			strncpy(path, pValue, pathSize - 1);
		} else { // error
			path[0] = 0;
		}
#endif
		//delete[] pValue;
	#else
        #error  need WIN32 or LIN_UX defined
	#endif
	path[pathSize - 1] = 0;
}
#endif

#if defined(WIN32) && defined (_DEBUG)
int xm_printf_vsdebug(const char * _Format, ...) {
	va_list args;
	char buf[256];
     va_start( args, _Format );
	int ok = vsnprintf(buf,255, _Format, args);
	if (ok > 0) { // we got the message
		OutputDebugStringA(buf);
		std::cout << buf;
	}
	va_end(args);
	return 0;
}
#endif

long long  findClosestPoint_1(long long  i1, long long i2, const double* v, double x) {
	long long i3;
	if (v == 0) {
		mxat(v != 0);
		return 0;
	}
	mxat(i2 > i1);
	if (x >= v[i2]) return i2;
	if (x <= v[i1]) return i1;

	while (i2 > (i1 + 1)) {
		i3 = (i1 + i2) >> 1;
		if (v[i3] > x) {
			i2 = i3;
		} else {
			i1 = i3;
		}
	}
	mxassert(((i1+1) >= i2) && (i2 >= i1), "");
	//if (x >= v[i2]) return i2;
	mxassert(x <= v[i2], "");
	mxassert(x >= v[i1], "");
	//if (x <= v[i1]) return i1;
	if ((v[i2] - x) < (x - v[i1])) {
		return i2;
	} else {
		return i1;
	}
}


#ifdef USING_BOOST_LIBRARY
#ifndef  BOOST_FILESYSTEM_FILESYSTEM_HPP
#include <boost/filesystem.hpp>
#endif

#include <boost/date_time/local_time_adjustor.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>


bool eexists(const std::string& fn) {
    using namespace boost::filesystem;
    bool e = false;
    try {
	   e = exists(fn);
    } catch(const filesystem_error& ex) {
	   e = false;
	   xm_printf("@WARNING: cannot access file %s (%s) ((no permission?)) \n",
		  fn.c_str(), ex.what());
    }

    return e;
}

boost::posix_time::ptime local_ptime_from_utc_time_t(std::time_t const t) {
    using boost::date_time::c_local_adjustor;
    using boost::posix_time::from_time_t;
    using boost::posix_time::ptime;
    boost::posix_time::ptime ret = c_local_adjustor<ptime>::utc_to_local(from_time_t(t));
    return ret;
}


#endif

int stripTrailingZeros(char* s) {
    int i = strlen(s);
    if(i == 0) {
	   return 0;
    }
	for (int j = 0; j < i; j++) {
		if ((s[j] == 69) || (s[j] == 101)) { // E or e
			return i;
		}
	}
    i--;
    //while ((i > 0) && ((s[i] > 57) || (s[i] < 49))) {
    while((i > 0) && (s[i] == 48)) {
	   i--;
    }
    if(i == 0) {
	   return 0;
    }
    if((s[i] == '.') || (s[i] == ',')) {
	   i++;
	   s[i] = '0';
    }
    s[i + 1] = 0;
    return i + 1;
}




/**    get    checksum.
*
* */
#define	ROT_LEFT(val) ((val << lShift) | (val >> rShift))
unsigned char getTLcs(unsigned char const* buf, int count) {
    enum {
	   bits = 8, lShift = 2, rShift = bits - lShift
    };
    unsigned char res = 0;
    while(count--) {
	   res = ROT_LEFT(res) ^ *buf++;
    }
    return ROT_LEFT(res);
}

CRC1::CRC1() {
	res = 0;
}
unsigned char CRC1::crc() {
	return ROT_LEFT(res);
}
void CRC1::start() {
	res = 0;
}
void CRC1::update(unsigned char const* buf, int count) {
	while(count--) {
		res = ROT_LEFT(res) ^ *buf++;
	}
}


static Crc16 const crc16Table[] = {
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
	0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
	0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
	0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
	0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
	0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
	0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
	0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
	0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
	0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
	0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
	0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
	0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
	0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
	0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
	0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
	0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
	0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
	0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
	0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
	0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
	0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
	0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
	0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
	0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
	0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
	0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
	0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
	0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};


CRC2::CRC2() {
   // crc16init();
    c0 = 0;
}
Crc16 CRC2::crc() {
    return c0;
}
/*
void CRC2::crc16init(void)	{
    Crc16 i;
    for(i = 0; i < TABLE_SIZE; ++i) {
	   Crc16 val = i << (WIDTH - BYTE_BITS);
	   int j;
	   for(j = 0; j < BYTE_BITS; ++j)
		  val = (val << 1) ^ ((val & MSB_MASK) ? POLY : 0);
	   table[i] = val;
    }
}
*/

Crc16 CRC2::crc16(Crc16 crc, void const* src, int cnt) {
    unsigned char const* s = (unsigned char const*)src;
    while(cnt--) {
	   crc = (crc << BYTE_BITS) ^
		   crc16Table[(crc >> (WIDTH - BYTE_BITS)) ^ *s++];
    }
    return crc;
}

void CRC2::update(unsigned char c) {
	c0 = (c0 << BYTE_BITS) ^ crc16Table[(c0 >> (WIDTH - BYTE_BITS)) ^ c];
}

void CRC2::update(const unsigned char* c, int count) {
    c0 =  crc16(c0, c, count);
}



