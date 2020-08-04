/**   round buffer.
	
	\file xmroundbuf.h
	\author   Igor Sandler
	\date   25 Dec 2006
	\version 1.0
	
*/

#ifndef ROUNDBUF_H_FILE
#define ROUNDBUF_H_FILE

//#include "xmatrixplatform.h"
#include "xstdef.h"


template <class T> class XMRoundBuffer {
public:
	int num;		///<   real number of elements
	XMRoundBuffer() { num = 0;  }
	virtual T& operator[](const int pos) = 0;
};

/**
	round buffer.
	T type of data.
	size    =   maximum size of data.

*/
template <class T, int count> class XMRoundBuf : public XMRoundBuffer<T> {
public:
	int read;		///<   read position
	int write;		///<   write position
	

	T	buf[count];  ///<    data 

public:
	XMRoundBuf() : XMRoundBuffer<T>() {
		read = 0;   write = 0;  
	}
	/**
		add   new element to the buffer.
	*/
	inline void	put(const T& x) {
		buf[write] = x;

		//  update indexes:
		++write; 
		if (write >= count) 
			write = 0;

		//   update number of elements
		/*if (num < count) {
			num++;
		}
		if (num == count) {
			read = write;
		}
		*/
		if (XMRoundBuffer<T>::num >= count) {
			read = write;
		} else {
			++XMRoundBuffer<T>::num;
		}
	}
	/**
		extract (and delete) last element from the buffer.
		@param[out]  x address   were to copy element
		@return true   if element was copyed
	*/
	inline bool	get(T* x = NULL) {
		if (XMRoundBuffer<T>::num == 0) {
			return false;	//   no info
		}

		if (x != NULL) {
			*x = buf[read];
		}

		++read; 
		XMRoundBuffer<T>::num--;
		if (read >= count)
			read = 0;
		return true;
	}
	/**
		obtain (and NOT delete)  element from the buffer.
		@param[out]  x address   were to copy element
		@param[in] pos position of the element.
		@return true   if element was copyed

	*/
	inline bool	peek(int pos, T* x) const {
		if (pos >= XMRoundBuffer<T>::num) {
			return false;
		}
		int i = read + pos;
		if (i >= count) {
			i -= count;
		}
		*x = buf[i];
		return true;
	}

	/**
		get a reference to a single element of the buffer.
		@param pos[in]  pos = num-1 => newest element.
	*/
	T& operator[](const int pos) {
		mxassert(pos < XMRoundBuffer<T>::num, "inline T& operator[]  error! \n");
		if(pos >= XMRoundBuffer<T>::num) {
			xm_printf("pos = %d;  num = %d ", pos, XMRoundBuffer<T>::num);

			return buf[0]; // ERROR!
		}

		int i = read + pos;
		if (i >= count) 
			i = i - count;
		return buf[i];
	}

	inline const T* operator()(int pos) const{
		mxassert(pos < XMRoundBuffer<T>::num, "inline const T* operator()  error! \n");
		if(pos >= XMRoundBuffer<T>::num) {
			return &(buf[0]); // ERROR!
		}

		int i = read + pos;
		if(i >= count)
			i = i - count;
		return &(buf[i]);
	}


	inline T operator[](const int pos) const {
		mxassert(pos < XMRoundBuffer<T>::num, "'inline T& operator[] const'  error! \n");
		if(pos >= XMRoundBuffer<T>::num) {
			return buf[0]; // ERROR!
		}

		int i = read + pos;
		if (i >= count) 
			i = i - count;
		return buf[i];
	}


	/**
		get const reference on the oldest element.
	*/
	inline const T& getFirstElement() const {
		mxassert(XMRoundBuffer<T>::num > 0, "inline const T& getFirstElement()  error! \n");

		return buf[read];
	}


	inline const T& getLastElement() const {
		mxassert(XMRoundBuffer<T>::num > 0, "inline const T& getLastElement()  error! \n");
		int k = write - 1;
		if(k < 0) {
			k = count - 1;
		}

		return buf[k];
	}



	/**
		
		@return true   or  false. 

	*/
	inline bool haveInfo() const {
		bool ret = true;
		if(XMRoundBuffer<T>::num == 0) {
			ret = false;
		}

		return ret;
	}

	/**
	
		@return true  .
	*/
	inline bool full() const { 
		bool ret = false;
		if(XMRoundBuffer<T>::num == count) {
			ret = true;
		}

		return ret;
	}
	/**
	
	@return real number of elements
  */
	inline int  getNum()  {	
		return XMRoundBuffer<T>::num; 
	}

	inline void empty() {
		read = 0;   write = 0;   XMRoundBuffer<T>::num = 0; 
	}

};



#define ACCURACYLIMIT1	100000
/**
	\class XSQBuf
		this round buffer can calculate it`s mean value 
		and sum of it`s elements.
*/

template <class T, unsigned int count> class XSQBuf : public XMRoundBuf<T, count> {
public:
	///   sum of all the elements.
	T sum;
	/// how many times sum was updated.
	unsigned int	sr;
public:
	XSQBuf<T, count>() : XMRoundBuf<T, count>() { 
		sum = 0.0;   
		sr = 0;
	}

	/**
		put new element.
	*/
	void	put(const T& x) {
		sum += x;
		if (XMRoundBuf<T, count>::num == count) {
			sum -= XMRoundBuf<T, count>::buf[XMRoundBuf<T, count>::write];
			sr++;
		}
		XMRoundBuf<T, count>::put(x);

		// How many times we change "sum"?
		if ((XMRoundBuf<T, count>::num == count) && (sr > ACCURACYLIMIT1)) {
			//   too many! we should recalculate it!  otherwise  accuracy will be low.
			sr = 0;
			sum = 0.0;
			int k;
			for (k = 0; k < count; k++) {
				sum += XMRoundBuf<T, count>::buf[k];
			}
		}
	}

	/**
		get element from the buffer 
		and remove it.
	*/
	bool	get(T* x = NULL) {
		sr++; // !
		if (XMRoundBuf<T, count>::num == 0) {
			return false;	//   no info
		}
		sum -= XMRoundBuf<T, count>::buf[XMRoundBuf<T, count>::read];
		return XMRoundBuf<T, count>::get(x);
	}

	/**
		get mean value of all the elements.
	*/
	T mean() {
		if (XMRoundBuf<T, count>::num < 1) {
			xm_printf("T mean() error! \n");
			return XMRoundBuf<T, count>::errorValue;
		}
		return (sum / XMRoundBuf<T, count>::num);
	}

	inline void empty() {
		XMRoundBuf<T, count>::read = 0;   XMRoundBuf<T, count>::write = 0;   XMRoundBuf<T, count>::num = 0; 
		    sr = 0;// sum = T();
			sum = 0.0;
	}

};



#endif



