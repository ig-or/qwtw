

/** very simple byte buffer.
	\file xqbytebuffer.h
	\author   Igor Sandler
	\date    Sep  2013
	\version 1.0

*/


#ifndef XQ_BYTE_BUFFER_H_FILE
#define XQ_BYTE_BUFFER_H_FILE

/** Byte buffer class.  This class is quite light.

*/
struct XQByteBuffer {
	XQByteBuffer(): swapFlag(false), buf(0), size(0), ms(0) {}
	/** create a byte buffer on particular data.
		@param[in] b our data
		@param[in] sf swap flag. If sf = true, we will change byte order then reading-writing bytes.
	*/
	XQByteBuffer(unsigned char*	b, bool sf = false): swapFlag(sf), buf(b), size(0), ms(0) {}
	XQByteBuffer(unsigned char*	b, int ms_, bool sf = false): swapFlag(sf), buf(b), size(0), ms(ms_) {}
	XQByteBuffer(char*	b, bool sf) : swapFlag(sf), buf((unsigned char*)(b)), size(0), ms(0) {}
	void setSwap(bool sf) { swapFlag = sf; }
	void set(unsigned char* x) { buf = x; size = 0;}
	/**  read out integer from byte buffer */
	int getInt();
	/** load info from buffer into array. 
		@param[out] v our destimation integer array
		@param[in] size size of 'v'
	*/
	void gi(int* v, int size);
	float getFloat();
	void gf(float* v, int size);
	double getDouble();
	void gd(double* v, int size);
	unsigned char	getUChar();

	void putUChar(unsigned char ch);
	void putInt(int x);
	void putString(const char* s);
	
	void putDPtr(double* x);
	double* getDPtr();
	void getString(char* s);
	unsigned int getSize() { return size; }

private:
	///  pointer to our buffer
	unsigned char*	buf;
	unsigned int size;
	int ms; ///< maximum size
	/// should we change bytes in numbers?
	bool swapFlag;
};


#endif
