/**
	very simple round buffer.
	contains  unsigned chars only.
*/

#ifndef RBUF_H_FILE
#define RBUF_H_FILE

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	unsigned char*	buf;
	int 	read; ///<   read position (from this pos I will read)
	int 	write; ///<   write position (to this position I will write)
	int 	num;  ///<   real number of elements
	int 	count; ///< maximum number of elements
	int	overflowCount;
} ByteRoundBuf;

void resetByteRoundBuf(ByteRoundBuf* b);
void initByteRoundBuf(ByteRoundBuf* b, unsigned char* src, int size);
void put_rb(ByteRoundBuf* b, unsigned char x);

/**  put  string into the buffer  
	@param[in] size size of 's'
*/
void put_rb_s(ByteRoundBuf* b, const unsigned char* s, int size);

/** read out string from a buffer.
	@return number of bytes in 's'

*/
int get_rb_s(ByteRoundBuf* b, unsigned char* s, int size);

/**
 	@return 0 if no info, 1 if x got an info
*/
int	get_rb(ByteRoundBuf* b, unsigned char*	x);

/**
 	@return 0 if no info, and one useful byte if b have an info
*/
unsigned char	get_rb_1(ByteRoundBuf* b);

/**   look at one element (but do not remove it from the buffer)
	@param[in] pos 0 - oldest element

	@return an element or 0 if pos is too big
*/
unsigned char	peek_rb(ByteRoundBuf* b, int pos);

//int	peek_rb_check(ByteRoundBuf* b, int pos, unsigned char* x);
	    
#ifdef __cplusplus
}
#endif

#endif




