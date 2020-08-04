

/** very simple byte buffer.
	\file xqbytebuffer.cpp
	\author   Igor Sandler
	\date    Sep  2013
	\version 1.0

*/


#include "xqbytebuffer.h"
#include "xstdef.h"
#include "xmutils.h"

#ifdef LIN_UX
#include <string.h>  ///  what for?
#endif


int XQByteBuffer::getInt() {
	int ret;
	if (swapFlag) {
		XM_INTBuf v;
		v.b[0] = buf[3];    v.b[1] = buf[2];  v.b[2] = buf[1];  v.b[3] = buf[0];
		ret =  v.x;
	} else {
		memcpy(&ret, buf, 4);
	}
	buf += 4;
	return ret;
}

void XQByteBuffer::putUChar(unsigned char ch) {
	buf[size] = ch;
	size += 1;
}

void XQByteBuffer::putInt(int x) {
	memcpy(buf + size, &x, sizeof(int));
	size += sizeof(int);
}

void XQByteBuffer::putString(const char* s) {
	if(s != 0) {
		int n = strlen(s);
		if(ms > 0) {
			int free = ms - size;
			mxassert(n < free, "");
			if(n >= free) {
				n = free;
			}
		}
		memcpy(buf + size, s, n); size += n;
	}
	buf[size] = 0; size += 1;
}

void XQByteBuffer::putDPtr(double* x) {
	memcpy(buf + size, &x, sizeof(double*));
	size += sizeof(double*);
}

double* XQByteBuffer::getDPtr() {
	double* ret;
	memcpy(&ret, buf, sizeof(double*));
	buf += sizeof(double*);
	return ret;
}

void XQByteBuffer::getString(char* s) {
	int n = strlen((const char*)buf);
	memcpy(s, buf, n); s[n] = 0;
	buf += n + 1;
}



float XQByteBuffer::getFloat() {
	float ret;
	if (swapFlag) {
		XM_FLBuf v;
		v.b[0] = buf[3];    v.b[1] = buf[2];  v.b[2] = buf[1];  v.b[3] = buf[0];
		ret =  v.x;
	} else {
		memcpy(&ret, buf, 4);
	}
	buf += 4;
	return ret;
}

double XQByteBuffer::getDouble() {
	double ret;
	if (swapFlag) {
		XM_DBLBuf v;
		v.b[0] = buf[7];    v.b[1] = buf[6];  v.b[2] = buf[5];  v.b[3] = buf[4];
		v.b[4] = buf[3];    v.b[5] = buf[2];  v.b[6] = buf[1];  v.b[7] = buf[0];		
		ret =  v.x;
	} else {
		memcpy(&ret, buf, 8);
	}
	buf += 8;
	return ret;
}

unsigned char	XQByteBuffer::getUChar() {
	unsigned char	c = buf[0];
	buf += 1;
	return c;
}

void XQByteBuffer::gi(int* v, int size) {
	for (int i = 0; i < size; i++) {
		v[i] = getInt();
	}
}
void XQByteBuffer::gf(float* v, int size) {
	for (int i = 0; i < size; i++) {
		v[i] = getFloat();
	}
}

void XQByteBuffer::gd(double* v, int size) {
	for (int i = 0; i < size; i++) {
		v[i] = getDouble();
	}
}



