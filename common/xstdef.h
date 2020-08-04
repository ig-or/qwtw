/** This file contain platform-specific definitions.

\file     xstdef.h
\author   Igor Sandler

\version 2.0



Possible defines:
==============================================
WIN32	: Windows
LIN_UX	: Linux
RDM1	: all the vehicle-related projects 
XIMU	: Topcon GNSS receiver
MATLAB_MEX_FILE : Matlab
SIMULINK		: Matlab Simulink
XM_ARM	: Cortex ARM M4    (only "float" single-precision numbers, 4 bytes)
XM_ARM_M0 : Cortex ARM M0 (no FPU at all)
=================================================

*/




#ifndef XMSTDEF_H_FILE
#define XMSTDEF_H_FILE

void assert_failed(const char* file, unsigned int line, const char* str);
#define XQX9STR(s) XQX8STR(s)
#define XQX8STR(s) #s

//#ifdef _DEBUG
	#define mxassert(expr, str) ((expr) ? (void)0 : assert_failed(__FILE__, __LINE__, str))
	#define mxat(expr)   ((expr) ? (void)0 : assert_failed(__FILE__, __LINE__, # expr))
//#else
	//#define mxassert(expr, str)
	//#define mxassert(expr, str) ((expr) ? (void)0 : assert_failed((unsigned char *)__FILE__, __LINE__, str))
//#endif

/**  many classes have this function implemented

*/
#define EC errorCheck(__FILE__, __LINE__)
#define EC2(st) errorCheck(st, __FILE__, __LINE__)

#ifdef WIN32  //    for winfows:
	#ifndef _CRT_SECURE_NO_WARNINGS
		#define _CRT_SECURE_NO_WARNINGS
	#endif
	#ifndef _CRT_SECURE_NO_DEPRECATE
		#define _CRT_SECURE_NO_DEPRECATE
	#endif
	//  disable XMType to float conversion warning:
	#pragma warning(disable:4244)
	#pragma warning(disable:4305)

//	#include <memory.h>
	#include <string.h>
	             
     #ifndef MATLAB_MEX_FILE    //   not matlab:
		#ifndef XMPRINTF_TO_FILE      //   printing not to file:
			#include <stdlib.h>
			#include <stdio.h>
//			#include <windows.h>
			#ifdef _DEBUG
#ifdef __cplusplus
				#include "xmutils.h"
				#define xm_printf xm_printf_vsdebug
#endif
			#else
				#define xm_printf printf
			#endif
		#endif
     #endif
	//#define getTimeStamp()    GetTickCount()

	// Teensy board emulation support: 
	#define micros() (GetTickCount() * 1000)
	#define millis() (GetTickCount())

	#define sprint xm_printf
	
#endif

#ifdef LIN_UX
	#include "memory.h"
	#include "stddef.h"
	#define strnicmp strncasecmp
	#include <stdexcept>
	#ifndef XMPRINTF_TO_FILE      //   printing not to file:
		#include <stdlib.h>
		#include <stdio.h>
		#define xm_printf printf
	#endif
#endif

#if defined (LIN_UX) || defined (WIN32) || defined (UNIX)

#define PC__VERSION 1
	#include <stdexcept>
	#ifndef XIMU_GR9
		#define XIMU_GR9 0
	#endif


#ifndef USE_VISOR
	
#else
	#define VS_FORMAT 2
	
	#define  USE_Z_GYRO_SCALE
#endif
	//  useful 'raise exception' macros:
	#define TLA(str) { char tlastmp[1024]; snprintf(tlastmp, 1023, "%s (file %s, line %d)", (str), (unsigned char *)__FILE__, __LINE__);  throw(std::logic_error(tlastmp)); }
	#define TLA2(str1, str2) { char tlastmp[1024]; snprintf(tlastmp, 1023, "%s %s (file %s, line %d)", (str1), (str2), (unsigned char *)__FILE__, __LINE__);  throw(std::logic_error(tlastmp)); }
	#define TCC(condition) { if (!(condition)) { char tlastmp[1024]; snprintf(tlastmp, 1023, \
			" %s (file %s, line %d)", \
			#condition, (unsigned char *)__FILE__, __LINE__); \
			 throw(std::logic_error(tlastmp)); } }
#ifndef _DEBUG
	#define TLL(condition, str) { if (condition) { char tlastmp[1024]; snprintf(tlastmp, 1023, " %s (file %s, line %d)", str, (unsigned char *)__FILE__, __LINE__);  throw(std::logic_error(tlastmp)); } }

#else
	#define TLL(condition, str) { if (condition) { char tlastmp[1024]; snprintf(tlastmp, 1023, " %s (file %s, line %d)", str, (unsigned char *)__FILE__, __LINE__);  mxat(false);  throw(std::logic_error(tlastmp)); } }
#endif
#endif

#ifdef RDM1
	#define useIVcorrection 1//  IMU->vehicle attitude correction
	#define useNHCcorrection 1
	#define useEncoderParamsCorrection 1
	#define useEarthRotation 1
	#define tstype long long
#else
	#define tstype int //  TODO: set this somewhere in project settings, in CMakeFile, together with s2ts
#endif

#ifdef XIMU  //    for receiver
	//#define tstype unsigned long 
	#define tstype int 
	//#define DPRINT_PORT 0 
	//#define Sleep Task::sleep
	#include "dprint.h"
	#define xm_printf DPRINT
	#include "memory.h"
	#ifdef APL_DUAL_HEADING
		#if  APL_DUAL_HEADING > 0
			#define  USE_VISOR
			#define  USE_Z_GYRO_SCALE
		#endif
	#endif
	//#define USE_IMU_RAW_DATA 1  

	// ~~~~~~~~~~~~~~~~~~~~~~~~
	//#define Tsc RcvTime
	//extern class RcvTimeScales RcvTime;	
	//#define getTimeStamp()     RcvTime.getTime(sv_GPS)
	//#include "ts.hpp"
#endif

#ifdef XMPRINTF_TO_FILE  // printing to file should be implemented separetely in some projects:
	//#include <stdio.h>
	//#define xm_printf(  fprintf(f_xmprintf,
	extern int xm_printf(const char * _Format, ...);
#endif

#ifdef MATLAB_MEX_FILE   //    working inside MATLAB:
		#ifdef SIMULINK     //    and inside SIMULINK:
			#include "simstruc.h"
			#define xm_printf  ssPrintf

#else     //    not SIMULINK, only MATLAB:
			#include <mex.h>
			#define xm_printf  mexPrintf
		#endif
#endif

// ######### (for ARM only?)  sometimes we'd like to call C code from C++ code: ###########
#ifdef XM_ARM_M0
	#define tstype int 
	#ifdef __cplusplus
		extern "C" {
	#endif
	//#include "stdlib.h"
	#include  "string.h"
	#ifdef __cplusplus
		}
	#endif
			
	//
	//#include "DLib_Product_string.h"
	#define XM_ARM
#endif

#ifdef XM_ARM
	//#define USE_SINGLE__PRECISION_NUMBERS
	#define tstype int 
	#ifdef __cplusplus
		extern "C" {
	#endif
	#include  "string.h"

	int xm_printf(const char * _Format, ...);  //  user-defined implementation
//#endif


//#ifdef XM_ARM
	#ifdef __cplusplus
		}
	#endif
#endif

// ###############################################################################################
/** 
		@return current PC time in [0.1 ms]; (getPCtime == 100)  => (PC time = 10 ms)
  */
//unsigned int getPCtime(); //   looks like we need this everywhere

#endif


