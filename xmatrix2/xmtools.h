
/**

	\file tools.h
	\author   Igor Sandler
	\date    nov 2006
	\version 1.0
	
*/


#ifndef XM_TOOLS_H_FILE2
#define XM_TOOLS_H_FILE2

#include "xmatrixplatform.h"

XMType  get42();


unsigned short int LSBF2MSBF_s(unsigned short int a);
short int LSBF2MSBF_s(short int a);
short int CE_CHORT(short int a);
int LSBF2MSBF(int a);


/** functions rotate bytes into motorolla format. */
//extern int xmToolswapFlag;
double char2dbl(unsigned char* c);
float char2float(unsigned char* c);
unsigned short int  char2short(unsigned char* c);
int char2int(unsigned char* c);

void dbl2char(const double x, unsigned char* c);
void float2char(const float x, unsigned char* c);
void int2char(const int x, unsigned char* c);
void short2char(const unsigned short int x, unsigned char* c);

/**     ��������� ����� ������ ��������.

*/
int strlen2(unsigned char* s);

/**    �������� ������ � ������ ������.

*/
void strcat2(unsigned char* s, const char* s1);

/**    ������������� ����� ����� � ������. 
	@param[in] value ����� �����
	@param[out] str ������, ���� ������� ���������.
	@param strsize ������ ������.

*/
void itoa2(int value, unsigned char* str, int strsize = 0);

/**    ������ ������� printf.

*/
void say(unsigned char* s, unsigned char* format, int x1=0, int x2=0, int x3=0, int x4=0);
/*
struct  WGS84 {
	/// WGS84  parameters.
	static const XMType	a;
	static const XMType	b; 
	static const XMType	f; 
	static const XMType	e; 
	static const XMType	es;
	///
	//	convert ECEF  into LLA.
	//	@param[in] ecef  XMType[3]: X, Y, Z
	//	@param[out]  LLA XMType[3]: lambda, fi, h (dolgota, shirota, visota)
	///
	static void ecef2LLA(XMType* ecef, XMType* LLA);
};
*/


/** calculate a median  of an array;
	@param[in] buf		our array
	@param[in] n number of elements in "buf"
	@return median

*/


//long	getMPC555Time();

//void printLogMessage(char* s, int x1=0, int x2=0, int x3=0, int x4=0);
//#ifndef WIN_PC
//#ifndef printf
//#ifndef MATLAB_MEX_FILE
//  ��� �������� ��� ����������.
//#define printf printLogMessage
//#endif
//#endif
//#endif


/*       
struct DebugInfoOut {
	static const int w = 32;
	static const int max = 32;
	int n;  //    number of items
	unsigned char item[max][w];

	void init();
	void addInfo(unsigned char* s);
	bool getInfo(unsigned char* s);
	void say(unsigned char* format, int x1=0, int x2=0, int x3=0, int x4=0);
	inline void say(char* format, int x1=0, int x2=0, int x3=0, int x4=0) {
		say((unsigned char*)format, x1, x2, x3, x4);
	}
};
extern DebugInfoOut debugInfoOut;
*/



#endif
