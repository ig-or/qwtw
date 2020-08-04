

/**
	\file tools.cpp
	\author   Igor Sandler
	\date    nov 2006
	\version 1.0
	
*/


#include "xmtools.h"
#include "xstdef.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h> //  FOR RAND_MAX	

#ifdef LIN_UX
#include <string.h>
#endif

XMType  get42() { return 42.0f; }   



int LSBF2MSBF(int a) {
	int ret = 0;
	ret += a & 0xff;
	ret <<= 8;
	ret += (a & 0xff00) >> 8;
	ret <<= 8;
	ret += (a & 0xff0000) >> 16;
	ret <<= 8;
	ret += (a & 0xff000000) >> 24;
	return ret;
}

unsigned short int LSBF2MSBF_s(unsigned short int a) {
	unsigned short int ret = 0;
	ret += a & 0xff;
	ret <<= 8;
	ret += (a & 0xff00) >> 8;
	return ret;
}

short int CE_CHORT(short int a) {
	unsigned short int ret = 0;
	ret += a & 0xff;
	ret <<= 8;
	ret += (a & 0xff00) >> 8;
	return ret;
}


//int xmToolswapFlag = 0;
union XM_DBLBuf {
	double x;
	unsigned char b[8];
};

union XM_FLBuf {
	float x;
	unsigned char b[4];
}; 

union XM_SHBuf {
	unsigned short int x;
	unsigned char b[2];
};

union XM_INTBuf {
	int x;
	unsigned char b[4];
};
// ====================================================================
double char2dbl(unsigned char* c) {
	XM_DBLBuf v;
	v.b[0] = c[7];    v.b[1] = c[6];  v.b[2] = c[5];  v.b[3] = c[4];
	v.b[4] = c[3];    v.b[5] = c[2];  v.b[6] = c[1];  v.b[7] = c[0];

	return v.x;
}


float char2float(unsigned char* c) {
	XM_FLBuf v;
	v.b[0] = c[3];    v.b[1] = c[2];  v.b[2] = c[1];  v.b[3] = c[0];

	return v.x;
}


unsigned short int  char2short(unsigned char* c) {
	XM_SHBuf v;
	v.b[0] = c[1];    v.b[1] = c[0]; 

	return v.x;
}

int char2int(unsigned char* c) {
	XM_INTBuf v;
	v.b[0] = c[3];    v.b[1] = c[2];  v.b[2] = c[1];  v.b[3] = c[0];
	return v.x;
}



void dbl2char(const XMType x, unsigned char* c) {
	XM_DBLBuf v;
	v.x = x;
	c[7] =v.b[0];    c[6] =v.b[1];  c[5] =v.b[2];  c[4] =v.b[3];
	c[3] =v.b[4];    c[2] =v.b[5];  c[1] =v.b[6];  c[0] =v.b[7];

}


void float2char(const float x, unsigned char* c) {
	XM_FLBuf v;
	v.x = x;
	c[3] =v.b[0];    c[2] =v.b[1];  c[1] =v.b[2];  c[0] =v.b[3];

}


void int2char(const int x, unsigned char* c) {
	XM_INTBuf v;
	v.x = x;
	c[3] =v.b[0];    c[2] =v.b[1];  c[1] =v.b[2];  c[0] =v.b[3];
}


void short2char(const unsigned short int x, unsigned char* c) {
	XM_SHBuf v;
	v.x = x;
	c[1] =v.b[0];  c[0] =v.b[1];

}




// ====================================================================

int strlen2(unsigned char* s) {
	int k = 0;
	while (s[k] != 0) {  k++; }
	return k;
}

void strcat2(unsigned char* s, const char* s1) {
	int k = 0, n;
	while (s[k] != 0) {  k++; }
	n = k;
	while (s1[k - n] != 0) { s[k] = s1[k-n]; k++; }
	s[k] = 0;
}

void strrev2(unsigned char* s) {
	unsigned char tmp;
	int k = 0;
	int j, i;
	while (s[k] != 0) { k += 1; }
	if (k < 2) { return; }
	for (i = 0, j = k - 1; i < j; i += 1, j -= 1) {
		tmp = s[j];
		s[j] = s[i];
		s[i] = tmp;
	}
}

void itoa2(int value, unsigned char* str, int strsize) {
	int rem = 0;
	int pos = 0;
	bool mflag = false;

	if (value == 0) {
		str[0] = '0'; str[1] = '\0'; return;
	}
	if (value < 0) {
		value = -value;
		mflag = true;
		if (strsize > 0) { strsize--; }
	}
	
	do {
		rem = value % 10;
		str[pos] = (unsigned char)(rem + 0x30);
		pos++;
		if (value == rem) { break; }
		value -= rem;
		value /= 10;

		if (value == 0) {str[pos] = '0'; pos++; break;	}

		if (value < 0) { str[0] = '&'; pos = 1; break; }  //   uncnown error;
		if ((strsize > 0) && (pos >= strsize -1)) { str[0] = '@'; pos = 1; break;	}   //   one more error;
	}  while (1);
	if (mflag) {
		str[pos] = '-'; pos++;
	}
	
	str[pos] = '\0';

	strrev2(str);
}

void say(unsigned char* s, unsigned char* format, int x1, int x2, int x3, int x4) {
	int i = 0;   //   format string counter;
	int k = 0;   //  s   string   counter;
	int arg = 0;	//   arg counter;
	int x = 0;	//   arg value;
	s[0] = 0;  // ?
	unsigned char q[32];
	int  j;
		
	while (format[i] != 0) {
		if (format[i] == '%') {
			i++;
			if (format[i] == 'd') {
				i++;
				arg++;
				j = 0;
				switch (arg) {
				case 1: 	x = x1;   break;
				case 2:		x = x2;   break;
				case 3:		x = x3;	  break;
				case 4:		x = x4;	  break;
				default:	x = 0;  //   error;
				};
				q[0] = 0;
				itoa2(x, q, 30);
				while (q[j] != 0) {  s[k] = q[j]; k++;  j++; }
				
			}  else  {  //   no 'd'    after '%' :
				s[k] = '%';     k++;
				s[k] = format[i]; k++;   i++;
			}
		}  else { //    f[i] != %:
			s[k] = format[i];
			k++;   i++;
		}
	}
	s[k] = 0;
}


