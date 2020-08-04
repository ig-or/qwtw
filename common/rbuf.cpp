/**
	very simple round buffer.
	contains  unsigned chars only.
*/


#include "rbuf.h"
#include "xstdef.h"

//#ifdef __cplusplus
//extern "C" {
//#endif

void initByteRoundBuf(ByteRoundBuf* b, unsigned char* src, int size) {
  	b->buf = src;
	b->count = size;
	resetByteRoundBuf(b);
}

void resetByteRoundBuf(ByteRoundBuf* b) {
	b->num = 0;
	b->read = 0;
	b->write = 0;
	b->overflowCount = 0;
}

void put_rb(ByteRoundBuf* b, unsigned char x) {
	mxassert(b != 0, "put_rb #1");
 	b->buf[b->write] = x;      //  save
	
	//  update indexes:
	if (b->write == b->read) { //  this was first element or overrun
		
		if (b->num == 0) { //  first element:
			b->num++;   //   update number of elements
		} else {     // overrun:
			mxassert(b->num == b->count, " put_rb ");
			b->read++;
			if (b->read == b->count) {
				b->read = 0;
			}
			b->overflowCount++;
		}
	} else {    // normal:
		b->num++;   //   update number of elements
	}
	
	b->write++; 
	if (b->write == b->count) {
		b->write = 0;
	}
}
				
int get_rb_s(ByteRoundBuf* b, unsigned char* s, int size) {
	int bs1, bs2 = 0;
	//mxassert((s != 0) && (size > 0) && (b != 0), "get_rb_s #1");
	if (b->num == 0) {
		return 0;
	}
	
	//  step #1:
	bs1 = b->count - b->read;  
	if (bs1 > 0) { // FIXME:  but this should always be true. max(read) = count-1
		if (bs1 > size) {
			 bs1 = size;
		}
		if (bs1 > b->num) {
			 bs1 = b->num;
		}
		memcpy(s, b->buf + b->read, bs1);
		if ((bs1 == size) || (bs1 == b->num)) { // finish #1
			b->read += bs1;
			if (b->read == b->count) {
				b->read = 0;
			}
			b->num -= bs1;
			return bs1;
		}
	
		size -= bs1;
		b->num -= bs1;
	}
	// step #2:
	//bs2 = b->read; 
	bs2 = b->write;
	if (bs2 > size) {
          bs2 = size;
	}
	//if (bs2 > b->num) { //  FIXME: we do not need this. here bs2 === num
	//	 bs2 = b->num;
	//}
	//mxassert(bs2 > 0, "get_rb_s #3");
	memcpy(s + bs1, b->buf, bs2);
	
	b->read = bs2;
	b->num -= bs2;

	return bs1 + bs2;
}
			
void put_rb_s(ByteRoundBuf* b, const unsigned char* s, int size) {
	int s1 = 0;
	int s2 = 0;
	int n;

#ifdef SAFE_MODE
	//mxassert((size <= b->count), "put_rb_s #1"); // size > count not supported
	//mxassert((size > 0), "put_rb_s #1.1");
	if (size <= 0) {
		resetByteRoundBuf(b);
		return;
	}
	if (size > b->count) {
		resetByteRoundBuf(b);
		return;
	}
#endif
    s1 = b->count - b->write;
	if (s1 >= size) {   //   this is very simple!
		s1 = size;
		memcpy(b->buf + b->write, s, s1);
	} else {
		
		memcpy(b->buf + b->write, s, s1);      // (*)
		s2 = size - s1;
#ifdef SAFE_MODE
		//mxassert(s2 > 0, "put_rb_s #2");
		if (s2 <= 0) {
			resetByteRoundBuf(b);
			return;
		}
#endif
		if (s2 > b->write) { // let's not overwrite "*"
			s2 = b->write;
		}
		if (s2 > 0) {
			memcpy(b->buf, s + s1, s2); 
		}
		
	}
	
	n = s1 + s2;
#ifdef SAFE_MODE
	//mxassert(n <= b->count, "put_rb_s #3");
	if (n > b->count) {
		resetByteRoundBuf(b);
		return;
	}
#endif
	
	b->write += n;
	if (b->write >= b->count) {
		b->write -= b->count;	
	}

	b->num += n;
	if (b->num > b->count) {
		b->num = b->count;
		b->read = b->write;
		++b->overflowCount;
	}
}			

int	get_rb(ByteRoundBuf* b, unsigned char*	x) {
	mxassert((x != 0) && (b != 0), "get_rb #1");
	if (b->num == 0) {
		mxassert((b->read == b->write), "get_rb #2");
		 return 0;  	//   no info
	}
	
	mxassert(b->read < b->count, "");
	 *x = b->buf[b->read];  // save
	 b->read++;

	 if (b->read == b->count) {
		 b->read = 0;
	 }

	 b->num--;
	 
#ifdef _DEBUG    //    do additional check:
	 if (b->num == 0) {
		  mxassert(b->read == b->write, "get_rb #3");
	 }
#endif
	 return 1;
}
	
unsigned char	get_rb_1(ByteRoundBuf* b) {
	unsigned char ret;
	if (b->num == 0) {
		 return 0;	//   no info
	 }

	mxassert(b->read < b->count, "");
	 ret = b->buf[b->read];
	
	 b->read++;

	 if (b->read == b->count) {
		 b->read = 0;
	 }

	b->num--;
	return ret;
}


unsigned char	peek_rb(ByteRoundBuf* b, int pos) {
	 int i;
	 mxassert((pos < b->num), "peek_rb #1");
	 if (pos >= b->num) {
		 return 0;
	 }
	 
	 i = b->read + pos;
	 if (i >= b->count)   {
		 i = i - b->count;
	 }
	 return b->buf[i];
}

//#ifdef __cplusplus
//}
//#endif



