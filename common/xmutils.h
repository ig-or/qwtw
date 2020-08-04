
/**
Utilitis      which do not belong to any calss.
\file    xmutils.h
\author   Igor Sandler

\version 1.0

*/



#ifndef XMUTILS_H_FILE1
#define XMUTILS_H_FILE1

// #include "xmatrixplatform.h"

// ===================== utils ========================

/** swap bytes in simple data types.
*/
template <typename T> inline void tswap(T& val) {
	char * p = (char *) &val;
	for ( unsigned i = 0; i < sizeof(T) / 2; i++ ) {
		char tmp = p[i];
		p[i] = p[sizeof(T) - i - 1];
		p[sizeof(T) - i - 1] = tmp;
	}
}


/** swap bytes in simple data types.
*/
inline void cswap(unsigned char* v, unsigned char size) {
	unsigned char s2 = size >> 1;
	for ( unsigned i = 0; i < s2; i++ ) {
		char tmp = v[i];
		v[i] = v[size - i - 1];
		v[size - i - 1] = tmp;
	}
}

#pragma pack(1)
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
#pragma pack()

/** test if parameter  is NAN or not

*/
bool rcvIsNan(float f);

#if defined (LIN_UX) || defined (WIN32)
/** Return path for current executable file.
	@param[out] path already allocated string
	@pathSize[in] size of path
*/
void getExeFilePath(char* path, int pathSize);
#endif

#ifdef WIN32
    #include <tchar.h>
    #include <string>
	/** string conversion: from  wchar_t to 'char'

	*/
	void w2str(wchar_t* src, char* dst);
	/** string conversion: from  TCHAR to 'char'

	*/
	void tchar2str(TCHAR* src, char* dst);

	/**  convert TCHAR into std::string
	*/
	std::string tchar2stdstring(TCHAR* src);
	#ifdef _DEBUG
		int xm_printf_vsdebug(const char * _Format, ...);
	#endif

    /**  @return path to temporal windows folder
    */
	std::string getTemporalPath();

	/**  @return path to 'common' windows folder (usually for all users)
	*/
	std::string getCommonAppDataPath();

	std::string getCurrentPath();

#endif

#ifdef LIN_UX
#include <string>

std::string getTemporalPath();

std::string getCommonAppDataPath();

#endif


/**  Division by 2 method. 
	Finds an index 'i' inside  'v', which is between 'i1' and 'i2' 
	such that 
	fabs(v[i] - x) -> min

	We assume that for every i1 and i2, 
	if i2 > i1 than v[i2] > v[i1]

	@return index for the point, closest to 'x'
	TODO: make it faster
*/
long long findClosestPoint_1(long long i1, long long i2, const double* v, double x);

/** remove zeros from the end of the string

*/
int stripTrailingZeros(char* s);

#ifdef USING_BOOST_LIBRARY
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>

/**
	@return true is file exists
	TODO: check if implementation is still OK
*/
bool eexists(const std::string& fn);
boost::posix_time::ptime local_ptime_from_utc_time_t(std::time_t const t);

#include <boost/circular_buffer.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/detail/call_traits.hpp>
//#include <boost/thread/condition.hpp>
//#include <boost/thread/thread.hpp>

template <class T> class bounded_buffer {
public:
	typedef boost::circular_buffer<T> container_type;
	typedef typename container_type::size_type size_type;
	typedef typename container_type::value_type value_type;
	typedef typename boost::call_traits<value_type>::param_type param_type;

	explicit bounded_buffer(size_type capacity) : m_container(capacity) {}

	void push(typename boost::call_traits<value_type>::param_type item) { // `param_type` represents the "best" way to pass a parameter of type `value_type` to a method.
		boost::mutex::scoped_lock lock(m_mutex);
		m_container.push_back(item);

		lock.unlock();
	}

	bool pop(value_type* pItem) {
		boost::mutex::scoped_lock lock(m_mutex);
		bool ret = false;
		if (!m_container.empty()) {
			//*pItem = m_container[--m_unread];
			*pItem = m_container.front();
			m_container.pop_front();
			return true;
		}
		lock.unlock();
		return ret;
	}

	int size() {
		boost::mutex::scoped_lock lock(m_mutex);
		int s = m_container.size();
		lock.unlock();
		return  s;
	}


private:
	bounded_buffer(const bounded_buffer&);              // Disabled copy constructor.
	bounded_buffer& operator=(const bounded_buffer&); // Disabled assign operator.

	container_type m_container;
	boost::mutex m_mutex;
}; //

#endif


/**  calculate a one byte checksum

@param[in] buf src buffer
@param[in] count  size of a buffer
@return a checksum
*/
unsigned char getTLcs(unsigned char const* buf, int count);

/** class for checksum calculation.

*/
class CRC1 {
public:
	CRC1();
	unsigned char crc();
	void start();
	void update(unsigned char const* buf, int count);
private:
	enum {
		bits = 8, lShift = 2, rShift = bits - lShift
	};
	unsigned char res;
};

/**  two-byte checksum

*/
typedef unsigned short Crc16;
class CRC2 {
public:
    CRC2();
    Crc16 crc();
    void start() {
	   c0 = 0;
    }
    void update(unsigned char c);
    void update(const unsigned char* c, int count);
    static Crc16 crc16(Crc16 crc, void const* src, int cnt);
private:
    Crc16 c0;
    enum {
	   WIDTH = 16, // Width of poly (number of most significant bits).
	   POLY = 0x1021, // Poly. Bit #16 is set and hidden.
	   BYTE_BITS = 8, // Number of bits in byte.
	   TABLE_SIZE = 1 << BYTE_BITS, // Size of table.
	   MSB_MASK = 1 << (WIDTH - 1) // Mask for high order bit in word.
    };

    // Table (generated by 'crc16init()'.
    //Crc16 table[TABLE_SIZE];

    // Initializes the table. Should be called once before the first call to
    // 'crc16()'
    
    //void crc16init(void);

    // Calculates CRC16 of 'cnt' bytes from 'src' and returns result.
    // Starting value of CRC16 is supplied by caller in 'crc'
    
};

#if defined (WIN32) || defined (UNIX)
#ifdef XQBUILDSYSTEM
/** obtain version info
	@param[out] vstr version info string
	@return number of bytes (?) in vstr

	warning: there is no any check for vstr size; 
*/
	//int xqversion(char* vstr);
	int xqversion(char* vstr, int bufSize);

#ifdef WIN32
	int xqversion(char* vstr, int bufSize, void* hModule);
#endif
#endif
#endif


#endif
