/**  platform-dependent constants 
 for matrix library only

 sometimes we need  'float', sometimes 'XMType'

 \file xmatrixplatform.h
 \author   Igor Sandler
 \date    Apr 2013
 \version 1.0


 */


#ifndef XMATRIXPLATFORM_H_FILE
#define XMATRIXPLATFORM_H_FILE

#include <float.h>
#include "xstdef.h"

#ifdef USE_SINGLE__PRECISION_NUMBERS
#ifndef PC__VERSION
	#ifndef SPN_FUNCTIONS_RENAME_5
		#define sin  sinf
		#define cos   cosf
		#define asin  asinf
		#define acos  acosf
		#define atan2 atan2f  
		#define  exp expf
		#define atan   atanf
		#define cosh coshf
		#define ceil ceilf
		#define floor   floorf
		#define fmod fmodf
		#define fabs   fabsf
		#define log logf
		#define sqrt   sqrtf
		#define lround lroundf
		#define SPN_FUNCTIONS_RENAME_5
	#endif
#endif

	#define pii	3.14159265358979f				///<   our "pi"
	#define pii05	1.57079632679489f	
	#define Rad2Deg 57.2957795130823f
	#define Deg2Rad 0.017453292519943295f


#ifdef _DEBUG
	#define DELTA_ZERO_1		FLT_EPSILON  //	1.192092896e-07F			///< Everything less than this is ZERO
#else
	#define DELTA_ZERO_1		FLT_EPSILON // 	
#endif
	#define VERYSMALLNUMBER (FLT_EPSILON * 2.0f)
	#define XMType	float   ///< internal matrix data type
	#define XMTSize 4		///< internal matrix data type size
	//#define iiType	 short  int 	  ///< data type  for index
	#define iiType	 int 	///< data type  for index

	#define ZERO		0.0f
	#define ONE		1.0f
	#define HALF		0.5f
	#define TWO		2.0f
	#define THREE	3.0f
	#define FORE		4.0f
	#define SIX		6.0f
	#define SEVEN	7.0f
	#define EIGHT	8.0f
	#define _xm_NAN 0.0f;
	#define BIGNUMBER FLT_MAX   //3.402823466e+38F  1E+36f

#else
	#define pii	3.14159265358979				///<   our "pi"
	#define pii05	1.5707963267948966		
	#define Rad2Deg 57.295779513082320876
	#define Deg2Rad 0.0174532925199432957692

	// DBL_EPSILON  ==   2.2204460492503131e-016 but this is too high for us
	#ifdef _DEBUG
		#define DELTA_ZERO_1		(DBL_EPSILON)				///<Everything less than this is ZERO
	#else
		#define DELTA_ZERO_1		(1.0e-12)	
	#endif
    #define VERYSMALLNUMBER (DBL_EPSILON * 4.0)

	#define XMType	double   ///< internal matrix data type
	#define XMTSize 8		///< internal matrix data type size
	#define iiType	 int 	  ///< data type  for index
	#define ZERO		0.0
	#define ONE		1.0
	#define HALF		0.5
	#define TWO		2.0
	#define THREE	3.0
	#define FORE		4.0
	#define SIX		6.0
	#define SEVEN	7.0
	#define EIGHT	8.0
	#define _xm_NAN ZERO;
	#define BIGNUMBER DBL_MAX    //   1.7976931348623158e+308   1e99
	
#endif

#define DZ1 DELTA_ZERO_1

#endif




