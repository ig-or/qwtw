
/** matrix library implementation

	\file xmatrix_imp.h
	\author   Igor Sandler
	\date    Jan 2008
	\version 2.0
	
*/



#ifndef MATRIX2_IMP_H_FILE
#define MATRIX2_IMP_H_FILE

#include "xstdef.h"

#ifdef PC__VERSION    
	#include <stdio.h>
	#include <stdlib.h>
#endif

#ifdef _DEBUG 
	#define NAMELEN_DBG	15						///<  ww of the name of an object (used for debug only)
#endif

#ifdef MATLAB_MEX_FILE
	#include "mex.h"
#endif

#ifdef SIMULINK // =================================================
	#include "simstruc.h"
#endif

#if 0
#ifdef ARDUINO
	#include "tbtools.h"
	#include "files.h"
#endif
#endif

#include <math.h>

#include "xmatrixplatform.h"

extern XMType d_error;

// ###############################################################################
// ###############################################################################
// ###############################################################################
// ###############################################################################

/** This class do almost all matris operations.
 It doesn`t contain matrix data - only pointers.
 Matrix data are in the wrapper  template.
 \class IMatrixImpl
*/
 class IMatrixImpl {
public:

	IMatrixImpl() {}

	// =====================================================================
#ifdef _DEBUG 
	/**  init "name" with _name.
	@param[in]	matrix name
	*/
	void init(const char* _name);  
#endif

#ifdef MATLAB_MEX_FILE
	void mxInit(const mxArray* mx);
	void toMXArray(mxArray* mx);
#endif

#ifdef SIMULINK

	/** fill in matrix data from simulink port.
		@param[in] S	simulink struct
		@param[in] port   block input port number
	*/
	void portInit(SimStruct *S, iiType port);

	/** feed simulink port with matrix data.
		@param[in] S	simulink struct
		@param[in] port   block output port number
	*/
	void	output(SimStruct *S, int_T port);
#endif

	/** fill matrix with zeros
	*/

	inline static void empty(XMType* x0, iiType h, iiType w) {
		memset(x0, 0, h*w*XMTSize); // may be this is not correct
	}

#ifdef PC__VERSION
	/** 	fill matrix with random numbers (for testing etc) .
	*/
	static void fillrnd(XMType* x0, iiType h, iiType w);
	static bool fromTextFile(char* fileName, XMType* x0, iiType h, iiType w);
	static bool fromTextFile(FILE* f, XMType* x0, iiType h, iiType w);
	static void saveToTextFile(FILE* f, const XMType* x0, iiType h, iiType w, int p = 24, char g = 'g');
	static void saveToTextFile(const char* fn, const XMType* x0, iiType h, iiType w, int p = 24, char g = 'g');
#endif

#if 0
#ifdef  ARDUINO
	static bool fromTextFile(char* fileName, XMType* x0, iiType h, iiType w);
	static bool fromTextFile(File& f, XMType* x0, iiType h, iiType w);
	static void saveToTextFile(File& f, const XMType* x0, iiType h, iiType w, int p = 24, char g = 'g');
	static void saveToTextFile(const char* fn, const XMType* x0, iiType h, iiType w, int p = 24, char g = 'g');
	//static void saveToBinaryFile(File& f, const XMType* x0, iiType h, iiType w, int p = 24, char g = 'g');
	//static void saveToBinaryFile(const char* fn, const XMType* x0, iiType h, iiType w, int p = 24, char g = 'g');

#endif
#endif


	/**  insert 1D matrix. 
		@param[in] m	matrix to insert 
		@param[in] from index from what element insert "m" into "this"
	*/
//	void insert1(const IMatrixImpl& m, unsigned short from);

	/**  insert 2D matrix .
		@param[in] m	matrix to insert 
		@param[in] fh index from what element insert "m" into "this" - row
		@param[in] fw index from what element insert "m" into "this" - column
	*/
	static void insert2(const IMatrixImpl& m, unsigned short  fh, unsigned short  fw);

	/**  add matrices.
		@param[in] m1	matrix to add
		@param[in] m2	matrix to add
		@param[out] dst result

	*/
	inline static void add(XMType* dst, const  XMType* m1, const XMType* m2, iiType size) {
		for(int i = 0; i < size; i++) {
			dst[i] = m1[i] + m2[i];
		}
	}

	/**  subtract matrices.
		@param[in] m1	matrix +
		@param[in] m2	matrix -
		@param[out] dst result

	*/
	inline static void sub(XMType* dst, const  XMType* m1, const XMType* m2, iiType size) {
		for(int i = 0; i < size; i++) {
			dst[i] = m1[i] - m2[i];
		}
	}

	/**  multiply matrices.
		TODO: May be this is not efficient way!!!!!    I will check this.
		@param[in] m1	matrix 1st
		@param[in] m2	matrix 2nd
		@param[out] dst result
	*/

	static void mul(XMType* dst, const XMType* m1, const XMType* m2, iiType hm1, iiType wm1, iiType wm2);


	/*		matrix transposition.	
			@param[out] dst result
	*/
	static void	transpose(XMType* dst, const  XMType* src, iiType h1, iiType w1);

	/** multiplication matrix by scalar. 
		@param d "scalar"
		@param[out] dst result
	*/
	//static void	mul(XMType* dst, const  XMType* src, XMType d, int size);

	static void	minus(XMType* dst, const  XMType*src, iiType size);

	/**   matrix root: get upper-triangular matrix.
	*/
	static bool utchol(XMType* dst, const XMType* src, iiType h, iiType w);
	/**    matrix root: get lower-triangular matrix.
	*/
	static bool ltchol(XMType* dst, const XMType* src, iiType h, iiType w);

	/** QR  algorithm (version #1).
	This algorithm can handle square matrices only.
	
	This is based on book "numerical recipes in C".
	void qrdcmp(float **a, iiType n, float *c, float *d, iiType *sing)
Constructs the QR decomposition of a[1..n][1..n]. The upper triangular matrix R is returned
in the upper triangle of a, except for the diagonal elements of R which are returned in
d[1..n]. The orthogonal matrix Q is represented as a product of n? 1 Householder matrices
Q1 . . .Qn?1, where Qj = 1?uj ?uj/cj. The ith component of uj is zero for i = 1, . . . , j ?1
while the nonzero components are returned in a[i][j] for i = j, . . . , n. sing returns as
true (1) if singularity is encountered during the decomposition, but the decomposition is still
completed in this case; otherwise it returns false (0).

Qj  � q1 -  used only for intermediate calculations, in case we need Q matrix.
 Arrays  c, d we need in any case.
	*/
	static bool qr(const XMType* src, XMType* R12, XMType* c, XMType* d, iiType h,
		XMType* Q = 0, XMType* Qj = 0, XMType* q1 = 0, XMType* u = 0);

#ifdef USE_GLS_LIBRARY
	/*  QR algorithm from GSL  library (version #2).  
		This algorithm can handle non-square matrices.
		\sa qr
	*/
	static bool qr_gsl(const XMType* src, XMType* R12, XMType* c, XMType* tau, iiType h, iiType w, XMType* Q = 0, XMType* u = 0, XMType* Qj = 0, XMType* q1 = 0);
#endif
	/**   matrix inversion (good, but not very efficient).   This worked unstable on a receiver platforms
	   (becouse of big stack usage?)
		@param M2 a result (inverted matrix)
		@param rowBuf a buffer for temporary information
		@return true if all is OK
	*/
	//bool	inv(IMatrixImpl& M2, XMType* rowBuf);

	/**    change two rows   
	*/
	void	changeRows(unsigned short  r1, unsigned short  r2, XMType* rowBuf);

	/**   square norm for a vector.
		  CPU: [size; size; 0]
	*/
	inline XMType norma2(const XMType* x, iiType size) const{
		 iiType i;  
		XMType n = ZERO;
		for (i = 0; i < size; i++) {
			n += x[i]*x[i];
		}
		return n;
	}

	/**   make vector size equal "1".

	*/
	void normalize(XMType* x, iiType size);
	void setLength(XMType* x, iiType size, XMType length);
	
public:
	///  install "1"  on main diagonal.
	inline static void make_eye(XMType* x, iiType h, iiType w) {
		 iiType i;  
		iiType  n = (h > w) ? w : h;

//		empty(x, h, w);
//#ifndef WIN32
//		for (i = 0; i < h*w; i++) { x[i] = ZERO; }
//#else
		memset(x, 0, h*w*XMTSize);  //  may be this is not correct
//#endif

		for (i = 0; i < n; i++) {
			x[i*w + i] = ONE;
		}
	}
	
	///  return matrix trace.
	inline static XMType trace(const XMType* x, iiType h, iiType w)  {
		iiType  n = (h > w) ? w : h;
		 iiType i;  

		XMType ret = ZERO;	
		for (i = 0; i < n; i++) {
			ret += x[i*w + i];	
		}	
		return ret;
	}

	static void dcm2qua_(XMType* q, const XMType* dcm);

#ifdef _DEBUG  
//	char	name[NAMELEN_DBG + 1];	///< name of our object;
#endif
	/**  print matrix for debug
		@param p number of digits after "."
	*/
	void print(const XMType* x, iiType hh, iiType ww, unsigned short  p) const; 

};



#endif


