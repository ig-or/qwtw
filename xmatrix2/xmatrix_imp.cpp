
/** matrix library implementation

	\file xmatrix_imp.cpp
	\author   Igor Sandler
	\date    Jan 2008
	\version 2.0
	
*/


//#define SIGNDIGITS 18		///< Maximal number of digits after the period
//#define NULL   0


//#include "xmatrixplatform.h"

#include "xmatrix_imp.h"

#include <stdio.h> //   for snprintf

#ifdef USE_GLS_LIBRARY // code from Dmitriy Ulkin
#include "xmgsl.h"
#endif

XMType d_error = 555.0; ///  If we have index error, we will return reference to this number

#ifdef _DEBUG 
#include <stdio.h>

void IMatrixImpl::init(const char* _name) {	
//	if (_name != 0) {
//		strncpy(name, _name, NAMELEN_DBG);
//	} else {
//		name[0] = 0; 
//	}  
}
#endif
/*
void IMatrixImpl::add(XMType* dst, const  XMType* m1, const XMType* m2, iiType size) {
	 iiType  i;
	for (i = 0; i < size; i++) {
		dst[i] = m1[i] + m2[i];
	}
}

void IMatrixImpl::sub(XMType* dst, const  XMType* m1, const XMType* m2, iiType size) {
	 iiType  i;
	for (i = 0; i < size; i++) {
		dst[i] = m1[i] - m2[i];
	}
}
*/
void	IMatrixImpl::minus(XMType* dst, const  XMType*src, iiType size) {
	 iiType  i;
	//  dimension check will be not here but in the upper class
	for (i = 0; i < size; i++) {
		dst[i] = -src[i];
	}
}

#ifdef PC__VERSION
void IMatrixImpl::fillrnd(XMType* x0, iiType h, iiType w) {
	int  i;
	int size = h * w;
	for (i = 0; i < size; i++) {
		x0[i] = ceil(XMType(((XMType)(rand())) / RAND_MAX * 4.0));
	}
}

void IMatrixImpl::saveToTextFile(FILE* f, const XMType* x0, iiType h, iiType w, int p, char g) {
	if (f == NULL) {
		xm_printf("\n IMatrixImpl::saveToTextFile: error; f == 0\n");
		return;
	}
	iiType i, j, k;
	char s2[20];
	snprintf(s2, 16, "%%.%dL%c ", p, g);

	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			k = fprintf(f, s2, (x0[j + i*w]));
		}
		fprintf(f, "\n");
	}
}

void IMatrixImpl::saveToTextFile(const char* fn, const XMType* x0, iiType h, iiType w, int p, char g) {
	FILE* f = fopen(fn, "wt");
	if (f == NULL) {
		xm_printf("\n IMatrixImpl::saveToTextFile: error; f == 0\n");
		return;
	}
	saveToTextFile(f, x0, h, w, p, g);
	fclose(f);
}


bool IMatrixImpl::fromTextFile(char* fileName, XMType* x0, iiType h, iiType w) {
	FILE* f = fopen(fileName, "rt");
	if (f == NULL) {
		xm_printf("\n IMatrixImpl::fromTextFile: error; can not open %s", fileName);
		return false;
	}
	bool ok = fromTextFile(f, x0, h, w);

	fclose(f);
	return ok;
}

bool IMatrixImpl::fromTextFile(FILE* f, XMType* x0, iiType h, iiType w) {
	if (f == NULL) {
		xm_printf("\n IMatrixImpl::fromTextFile: error; f == 0\n");
		return false;
	}
	iiType i, j, k;
	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
#ifdef USE_SINGLE__PRECISION_NUMBERS
			k = fscanf(f, "%g", &(x0[j + i*w]));
#else
			k = fscanf(f, "%Lg", &(x0[j + i*w]));
#endif
			if (k != 1) {
				//error
				memset(x0, 0, h*w*XMTSize);
				xm_printf("\n IMatrixImpl::fromTextFile: format error; i=%d; j = %d",
					i, j);

				return false;
				break;
			}
		}
		if (k != 1) break;
	}
	return true;
}

#endif

#if 0
#ifdef  ARDUINO

void IMatrixImpl::saveToTextFile(File& f, const XMType* x0, iiType h, iiType w, int p, char g) {
	//if (f == NULL) {
	//	xm_printf("\n IMatrixImpl::saveToTextFile: error; f == 0\n");
	//	return;
	//}
	iiType i, j, k;
	char s2[20];
	//char s3[32];
	//char s4[128];
	snprintf(s2, 16, "%%.%dL%c ", p, g);

	for (i = 0; i < h; i++) {
		//s4[0] = 0;
		for (j = 0; j < w; j++) {
			k = f.printf(s2, (x0[j + i*w]));
			//snprintf(s3, 32, s2, x0[j + i*w]); s3[31] = 0;
			//strcat(s4, s3);
		}
		//f.printf("%s\n", s4);
		f.printf("\n");
	}
}

void IMatrixImpl::saveToTextFile(const char* fn, const XMType* x0, iiType h, iiType w, int p, char g) {
	//if (SD.exists(fn)) {
	//	SD.remove(fn);
	//}

	File f = SD.open(fn, O_WRITE | O_CREAT | O_TRUNC);

	//if (f == NULL) {
	//	xm_printf("\n IMatrixImpl::saveToTextFile: error; f == 0\n");
	//	return;
	//}
	saveToTextFile(f, x0, h, w, p, g);
	f.close();
}


bool IMatrixImpl::fromTextFile(char* fileName, XMType* x0, iiType h, iiType w) {
	//FILE* f = fopen(fileName, "rt");
	File f = SD.open(fileName, FILE_READ);

	//if (f == NULL) {
	//	xm_printf("\n IMatrixImpl::fromTextFile: error; can not open %s", fileName);
	//	return false;
	//}
	bool ok = fromTextFile(f, x0, h, w);

	f.close();
	return ok;
}
/*
bool str2f__a(char** pch, XMType& f) {
	char* endptr;
	bool ok = true;
	if (*pch != 0) {
#ifdef USE_SINGLE__PRECISION_NUMBERS
		f = strtof(*pch, &endptr);
#else
		f = strtod(*pch, &endptr);
#endif
		*pch = strtok(NULL, " ");
		return ok;
	} else { //  error
		return false;
	}
}

*/

bool IMatrixImpl::fromTextFile(File& f, XMType* x0, iiType h, iiType w) {
	//if (f == NULL) {
	//	xm_printf("\n IMatrixImpl::fromTextFile: error; f == 0\n");
	//	return false;
	//}
	iiType i, j, k;
	const int ls = 128;
	char line[ls];
	char* pch;
	int u = 0, bs;
	
	for (i = 0; i < h; i++) {
		//String s(f.readString());
		//line = s.c_str();
		//strncpy(line, s.c_str(), 128);
		for (u = 0; u < (ls - 2); u++) {
			//line[u] = f.read(); 
			bs = f.read(line + u, 1);
			if (bs <= 0) {
				xm_printf("\n IMatrixImpl::fromTextFile: format error 0; bs = %d i=%d; j = %d; k = %d; h = %d;  w = %d; u = %d; line = {%s}\n",
					bs, i, j, k, h, w, u, line);
				return false;
			}

			if (line[u] == '\n') {
				break;
			}
		}
		line[u] = 0;

		pch = strtok(line, " ");
		for (j = 0; j < w; j++) {
			if ((pch == 0) || (pch[0] == 0)) {
				xm_printf("\n IMatrixImpl::fromTextFile: format error 1; i=%d; j = %d; k = %d; h = %d;  w = %d; u = %d; line = {%s}\n",
					i, j, k, h, w, u, line);
				return false;
			}
#ifdef USE_SINGLE__PRECISION_NUMBERS
			//k = fscanf(f, "%g", &(x0[j + i*w]));
			k = sscanf(pch, "%g", &(x0[j + i*w]));
#else
			//k = fscanf(f, "%Lg", &(x0[j + i*w]));

			k = sscanf(pch, "%Lg", &(x0[j + i*w]));
#endif
			pch = strtok(NULL, " ");
			if (k != 1) {
				//error
				memset(x0, 0, h*w*XMTSize);
				xm_printf("\n IMatrixImpl::fromTextFile: format error 2; i=%d; j = %d; k = %d; h = %d;  w = %d; u = %d;  line = {%s}\n",
					i, j, k, h, w, u, line);

				return false;
				break;
			}
		}
		if (k != 1) break;
	}
	return true;
}

#endif
#endif


//#ifdef _DEBUG
void IMatrixImpl::print(const XMType* x, iiType hh, iiType ww, unsigned short  p) const  {
	unsigned short  i,j;
	char	s1[128];
	char	s3[36];
	char	s2[20];

//#ifdef _DEBUG
//	if (name[0] == 0) {
//		//strncpy(name, "noname", NAMELEN_DBG);
//		snprintf(s1, 32, "matrix   %d x %d :  \n",  hh, ww);
//	} else {
//		snprintf(s1, 32, "matrix  %s %d x %d :  \n", name, hh, ww);
//	}
//#else
	snprintf(s1, 32, " matrix   %d x %d :  \n", hh, ww);
//#endif

	xm_printf(s1);
				
	snprintf(s2, 16, "\t%%.%dg", p);
//#ifdef _DEBUG
//	xm_printf("%s = [", name);
//#else
	xm_printf("[");
//#endif
	for(i=0;i<hh;i++) {
		s1[0] = 0;
		for(j=0;j<ww;j++) {
			snprintf(s3, 32, s2, x[i*ww + j]);
			strcat(s1, s3); 
			if (j != (ww - 1)) {
				strcat(s1, "  ");
			} 
		}
		if (i == (hh - 1)) { strcat(s1, "];");} else { strcat(s1, ";"); }
		xm_printf("%s\n", s1);
	}
	xm_printf("\n"); 
}
//#endif


void IMatrixImpl::mul(XMType* dst, const XMType* m1, const XMType* m2, iiType hm1, iiType wm1, iiType wm2) {
	 iiType i;
	 iiType j;
	 iiType k;

	for(i=0; i<hm1; i++)		for(j=0; j<wm2; j++){
		dst[i*wm2+j] = ZERO;
		for(k=0; k<wm1; k++) {
			dst[i*wm2+j] += m1[i*wm1+k] * m2[k*wm2+j];
		}
	}
}

void IMatrixImpl::normalize(XMType* x, iiType size) {
	 iiType i;
	XMType n = ZERO;
	for(i = 0; i < size; i++) {
		n += x[i] * x[i];
	}
	if(fabs(n - ONE) <= DELTA_ZERO_1) {
		return;
	}

	mxassert(n >= DELTA_ZERO_1, "normalize faild"); //  vector size too small
	if(n <= DELTA_ZERO_1) {
		//xm_printf();
		return;
	}

	n = sqrt(n);

	for(i = 0; i < size; i++) {
		x[i] /= n;
	}
}

void IMatrixImpl::setLength(XMType* x, iiType size, XMType length) {
	 iiType i;
	XMType n = ZERO;
	for (i = 0; i < size; i++) {
		n += x[i] * x[i];
	}

	mxassert(n >= DELTA_ZERO_1, "setLength faild"); //  vector size too small
	if (n <= DELTA_ZERO_1) {
		//xm_printf();
		return;
	}

	n = length / sqrt(n);

	for (i = 0; i < size; i++) {
		x[i] *= n;
	}
}


void	IMatrixImpl::transpose(XMType* dst, const  XMType* src, iiType h1, iiType w1) {
	iiType  i, j;
	for (i = 0; i < h1; i++) for (j = 0; j < w1; j++) {
		dst[j*h1+i] = src[i*w1 + j];
	}
}

/*
void	IMatrixImpl::changeRows(unsigned short  r1, unsigned short  r2, XMType* rowBuf) {
#ifdef _DEBUG
	if ((r1 >= h0) || (r2 >= h0)) {
		xm_printf("IMatrixImpl::changeRows error \n ");
		return;
	}
#endif
	unsigned short  rowsize = w0 * sizeof(XMType);
	memcpy(rowBuf, x0 + r1*w0, rowsize);
	memcpy(x0 + r1*w0, x0 + r2*w0, rowsize);
	memcpy(x0 + r2*w0, rowBuf, rowsize);
}

*/
bool IMatrixImpl::utchol(XMType* dst, const XMType* src, iiType h, iiType w) {
	int i,j,k;
	int size = h*w;
	int itmp;
	XMType sigma;
	// ? iiType m = (w0 < h0) ? w0 : h0;
	mxassert(w == h, "IMatrixImpl::utchol:  matrix is not square ");
	if (w != h) {
		return false;   //   matrix is not square 
	}
	for (itmp = 0; itmp < size; itmp++) { dst[itmp] = ZERO; }//dst.empty();
	for (j = w - 1; j >= 0; j--) {
		for (i = j; i >= 0; i--) {
			sigma = src[i*w + j];
			for (k=j+1; k < w; k++) 
				sigma -= dst[i*w + k]*dst[j*w + k];
			dst[j*w+i] = ZERO;
			if (i == j) {
				mxassert(sigma > ZERO, "IMatrixImpl::utchol: sigma <= ZERO ");
				if (sigma <= ZERO)  {// src, with rounding errors, is not positive definite.
#ifdef _DEBUG
					return false;
#else
					dst[i*w + j] = ZERO;
#endif
				} else {
					dst[i*w + j] = sqrt(sigma);
				}
			} else {
				if (dst[j*w + j] == ZERO)
					dst[i*w + j] = ZERO;
				else
					dst[i*w + j] = sigma / dst[j*w + j];
			}
		}
	}
	return true;
}

bool IMatrixImpl::ltchol(XMType* dst, const XMType* src, iiType h, iiType w) {
	int i,j,k;
	XMType sigma;
	int size = h*w;
	int itmp;
	iiType m = (w < h) ? w : h;
	mxassert(w == h, " ");
	if (w != h) {
		return false;   //   matrix is not square 
	}
	for (itmp = 0; itmp < size; itmp++) { dst[itmp] = ZERO; }//dst.empty();
	for (j = 0; j < m; j++) {
		for (i = j; i < m; i++) {
			sigma = src[j*w + i];
			for (k = 0; k <= (j-1); k++) 
				sigma -= dst[i*w + k]*dst[j*w + k];
			dst[j*w+i] = ZERO;
			if (i == j) {
				mxat(sigma > ZERO);
				if (sigma <= ZERO)  {// x0, with rounding errors, is not positive definite.
#ifdef _DEBUG
					return false;
#else
					dst[i*w + j] = ZERO;
#endif
				} else {
					dst[i*w + j] = sqrt(sigma);
				}
			} else {
				if(fabs(dst[j*w + j]) < VERYSMALLNUMBER)
					dst[i*w + j] = ZERO;
				else
					dst[i*w + j] = sigma / dst[j*w + j];
			}
		}
	}
	return true;
}

bool IMatrixImpl::qr(const XMType* src, XMType* R12, XMType* c, XMType* d, iiType h,	XMType* Q, XMType* Qj, XMType* q1, XMType* u)  {
	int i, j, k, m;
	int n = h;
	int size = h*h;

	//for (i = 0; i < size; i++) R12.x0[i] = x0[i];   //   this
	memcpy(R12, src, size*XMTSize);

	// ========================    start of NRC  ====================
	XMType scale,sigma,sum,tau;
	bool ok = true;
	for (k=0; k < (n-1); k++) {
		scale=ZERO;
		for (i = k; i < n; i++) scale=(scale > fabs(R12[i*h+k])) ? scale : fabs(R12[i*h+k]);
		if (scale == ZERO) { //Singular case.
			ok = false;
			c[k]=d[k]=ZERO;
		} else { //Form Qk and Qk � A.
			for (i = k; i < n; i++) R12[i*h+k] /= scale;
			for (sum = ZERO, i = k; i < n; i++) sum += R12[i*h+k]*R12[i*h+k];
			sigma=sqrt(sum);
			if (R12[k*h+k] < 0) sigma = -sigma;

			R12[k*h+k] += sigma;
			c[k]=sigma * R12[k*h+k];
			d[k] = -scale * sigma;
			for (j = k+1; j < n; j++) {
				for (sum=ZERO, i=k; i < n; i++) 	sum += R12[i*h+k]*R12[i*h+j];
				tau = sum / c[k];
				for (i=k; i <n; i++)  R12[i*h+j] -= tau*R12[i*h+k];
			}
		}
	}
	d[n-1]=R12[(n-1)*h+(n-1)];
	if (d[n-1] == ZERO) ok = false;

	// =====================   end of NRC  =========================================


	if ((Q == 0) || (Qj == 0) || (q1 == 0) || (u == 0))  { // ���� ������� Q �� �����, �� �����.
		//  ================== create R12: =====================
		for (i = 0; i < n; i++) R12[i*h+i] = d[i];
		for (i = 0; i < n; i++) for (j = 0; j < i; j++) R12[i*h+j] = ZERO;

		// set all diag elements > 0
		for (i = 0; i < n; i++) {
			if (R12[i*h + i] < ZERO) {
				for (j = i; j < n; j++) R12[i*h+j] *= -ONE;
			}
		}
		return ok;
	}

	// ================== create Q: =========================================
	memset(Q, 0, size*XMTSize);     //Q->empty();
	for (i = 0; i < n; i++) Q[i*h+i] = ONE;
	for (j = 0; j < (n-1); j++) {
		for (i = 0; i < j; i++) u[i] = ZERO;
		for (i = j; i < n; i++) u[i] = R12[i*h + j];
		for (i = 0; i < n; i++) for (k = 0; k < n; k++) Qj[i*h+k] = -u[i]*u[k] / c[j];
		for (i = 0; i < n; i++) Qj[i*h+i] += ONE;

		for (i = 0; i < n; i++) for (k = 0; k < n; k++) q1[i*h+k] = Q[i*h+k];
		memset(Q, 0, size*XMTSize);     //Q->empty();
		for (i = 0; i < n; i++) for (k = 0; k < n; k++) for (m = 0; m < n; m++) {
			Q[i*h+k] += q1[i*h+m] * Qj[m*h + k];
		}
	}

	//  ================== create R12: =====================
	for (i = 0; i < n; i++) R12[i*h+i] = d[i];
	for (i = 0; i < n; i++) for (j = 0; j < i; j++) R12[i*h+j] = ZERO;

	// set all diag elements > 0
	for (i = 0; i < n; i++) {
		if (R12[i*h + i] < ZERO) {
			for (j = i; j < n; j++) R12[i*h+j] *= -ONE;
			for (j = 0; j < n; j++) Q[j*h + i] *= -ONE;
		}
	}
	return ok;
}

#ifdef USE_GLS_LIBRARY

bool IMatrixImpl::qr_gsl(const XMType* src, XMType* R12, XMType* c, XMType* tau, iiType h, iiType w, XMType* Q, XMType* u, XMType* Qj, XMType* q1) {
	int i, j, k, m;
	int size = h*w;
	
	//for (i = 0; i < size; i++) R12.x0[i] = x0[i];   //   this
	memcpy(R12, src, size*XMTSize);
	iiType gm = (h < w) ? h : w;

	int ret = gsl_linalg_QR_decomp(R12, h, w, tau, gm, c);
	ret = ret;  // ?

	if ((Q == 0)) {
		//  ================== create R12: =====================
		for (i = 0; i < h; i++) for (j = 0; ((j < i)&&(j < w)); j++) 
			R12[i*w+j] = 0.;

		// set all diag elements > 0
		for (i = 0; i < gm; i++) {
			if (R12[i*w + i] < 0) {
				for (j = i; j < w; j++) R12[i*w+j] *= -ONE;
			}
		}
		return true;
	}

	// ================== create Q: =========================================
	// ��������� Q
	memset(Q, 0, size*XMTSize);     //Q->empty();
	for (i = 0; i < h; i++) Q[i*h+i] = ONE;

	// ������� Q
	//for (i = gm-1; i >= 0; i--) {
	for (i = 0; i <gm ; i++) {
		for (j = 0; j < i; j++) u[j] = ZERO;
		u[i] = 1.;
		for (j = i+1; j < h; j++) u[j] = R12[i + j*w];

		for (j = 0; j < h; j++) for (k = 0; k < h; k++) Qj[k + j*h] = -tau[i] * u[j] * u[k];

		for (j = 0; j < h; j++) Qj[j*h + j] += ONE;

		for (j = 0; j < h; j++) for (k = 0; k < h; k++)    q1[k + j*h] = Q[k + j*h];

		memset(Q, 0, size*XMTSize);     //Q->empty();
		for (j = 0; j < h; j++) for (k = 0; k < h; k++) for (m = 0; m < h; m++) {

			Q[j*h + k] += q1[j*h + m] * Qj[m*h + k];
		}
	}

	//  ================== create R12: =====================
	for (i = 0; i < h; i++) for (j = 0; ((j < i)&&(j < w)); j++) 
		R12[i*w+j] = 0.;

	// set all diag elements > 0
	for (i = 0; i < gm; i++) {
		if (R12[i*w + i] < 0) {
			for (j = i; j < w; j++) R12[i*w+j] *= -ONE;
			for (j = 0; j < h; j++) Q[j*h + i] *= -ONE;
		}
	}

	return true;
}

#endif

/** init quaternion from DCM  */
void IMatrixImpl::dcm2qua_(XMType* q, const XMType* dcm) {
	q[3] = ZERO;
	XMType tr = IMatrixImpl::trace(dcm, 3, 3);
	if (tr > ZERO) {
        XMType sqtrp1 = sqrt( tr + ONE );
        
        q[0] = HALF*sqtrp1; 
        q[1] = (dcm[1*3+2] - dcm[2*3+1]) / (2.0*sqtrp1);
        q[2] = (dcm[2*3+0] - dcm[0*3+2]) / (2.0*sqtrp1); 
        q[3] = (dcm[0*3+1] - dcm[1*3+0]) / (2.0*sqtrp1); 

	} else {
		XMType sqdip1;
		XMType d1 = dcm[0], d2 = dcm[1*3+1], d3 = dcm[2*3+2];  //   diagonal elements
		if ((d2 > d1) && (d2 > d3)) { // d2  maximum
			sqdip1 = sqrt(d2 - d1 - d3 + ONE );
            
            q[2] = HALF*sqdip1; 
            
            if ( sqdip1 >  DELTA_ZERO_1 ) {
                sqdip1 = HALF / sqdip1;
			}
            
            q[0] = (dcm[2*3+1] - dcm[0*3+2])*sqdip1; 
            q[1] = (dcm[0*3+2] + dcm[1*3+0])*sqdip1; 
            q[3] = (dcm[1*3+2] + dcm[2*3+1])*sqdip1; 
		} else if (d3 > d1) {  //    d3 maximum
			sqdip1 = sqrt(d3 - d1 - d2 + ONE );
            
            q[3] = HALF*sqdip1; 
            
            if ( sqdip1 >  DELTA_ZERO_1 ) {
                sqdip1 = HALF/sqdip1;
			}
            
            q[0] = (dcm[0*3+1] - dcm[1*3+0])*sqdip1;
            q[1] = (dcm[2*3+0] + dcm[0*3+2])*sqdip1; 
            q[2] = (dcm[1*3+2] + dcm[2*3+1])*sqdip1; 

		} else { //    d1 maximum
			 sqdip1 = sqrt(d1 - d2 - d3 + ONE );
            
            q[1] = HALF*sqdip1; 
            
            if ( sqdip1  >  DELTA_ZERO_1 ) {
                sqdip1 = HALF/sqdip1;
			}
            
            q[0] = (dcm[1*3+2] - dcm[2*3+1])*sqdip1; 
            q[2] = (dcm[0*3+1] + dcm[1*3+0])*sqdip1; 
            q[3] = (dcm[2*3+0] + dcm[0*3+2])*sqdip1; 
		}
	}
	//if (q[0] < ZERO) {
	//	for(int i = 0; i < 4; i++) q[i] = -q[i];
	//}
}

#ifdef MATLAB_MEX_FILE
void IMatrixImpl::mxInit(const mxArray* mx) {
	mxClassID cid = mxGetClassID(mx);
	size_t h1 = mxGetM(mx);
	size_t w1 = mxGetN(mx);
	//  check bounds:
	if ((h1 != h0) || (w1 != w0)) {
		mexPrintf("IMatrixImpl::mxInit(): bounds check error! \n");
		return;
	}
	iiType i;  // row
	iiType j;  // column

	XMType*			dbuf;
	unsigned char*	ucBuf;
	iiType*			iBuf;
	unsigned iiType*	uiBuf;
	unsigned short iiType*	usiBuf;

	switch(cid) {
		case mxDOUBLE_CLASS:  
			dbuf = (XMType *)(mxGetPr(mx));
			for (i = 0; i < h0; i++) for (j = 0; j < w0; j++) { 
				x0[j + i*w0] = ((XMType)(dbuf[i + j*h0])); 
			} 
			break;
		case mxUINT8_CLASS:  
			ucBuf = (unsigned char*) mxGetData(mx);
			for (i = 0; i < h0; i++) for (j = 0; j < w0; j++) { 
				x0[j + i*w0] = ((XMType)(ucBuf[i + j*h0])); 
			} 
			break;
		case mxINT32_CLASS:  
			iBuf = (iiType*) mxGetData(mx);
			for (i = 0; i < h0; i++) for (j = 0; j < w0; j++) { 
				x0[j + i*w0] = ((XMType)(iBuf[i + j*h0])); 
			} 
			break;
		case mxUINT32_CLASS:  
			uiBuf = (unsigned iiType*) mxGetData(mx);
			for (i = 0; i < h0; i++) for (j = 0; j < w0; j++) { 
				x0[j + i*w0] = ((XMType)(uiBuf[i + j*h0])); 
			} 
			break;
		case mxUINT16_CLASS:  
			usiBuf = (unsigned short iiType*) mxGetData(mx);
			for (i = 0; i < h0; i++) for (j = 0; j < w0; j++) { 
				x0[j + i*w0] = ((XMType)(usiBuf[i + j*h0])); 
			} 
			break;

		default:
			mexPrintf("IMatrixImpl::mxInit(): data type unsupported! \n");

	};
}

void IMatrixImpl::toMXArray(mxArray* mx) {
	mxClassID cid = mxGetClassID(mx);
	size_t h1 = mxGetM(mx);
	size_t w1 = mxGetN(mx);
	//  check bounds:
	if ((h1 != h0) || (w1 != w0)) {
		mexPrintf("IMatrixImpl::toMXArray(): bounds check error! \n");
		return;
	}
	iiType i;  // row
	iiType j;  // column

	XMType*			dbuf;
	unsigned char*	ucBuf;
	iiType*			iBuf;
	unsigned iiType*	uiBuf;
	unsigned short iiType*	usiBuf;

	switch(cid) {
		case mxDOUBLE_CLASS:  
			dbuf = (XMType *)(mxGetPr(mx));
			for (i = 0; i < h0; i++) for (j = 0; j < w0; j++) { 
				dbuf[i + j*h0] = ((XMType)(x0[j + i*w0]));
			} 
			break;
		case mxUINT8_CLASS:  
			ucBuf = (unsigned char*) mxGetData(mx);
			for (i = 0; i < h0; i++) for (j = 0; j < w0; j++) { 
				if (x0[j + i*w0] < ZERO) { 
					ucBuf[i + j*h0] = 0; 
				} else {
					if (x0[j + i*w0] > 255.) { 
						ucBuf[i + j*h0] = 255; 
					} else {
						ucBuf[i + j*h0] = ((unsigned char)(x0[j + i*w0]));
					}
				}
			} 
			break;
		case mxINT32_CLASS:  
			iBuf = (iiType*) mxGetData(mx);
			for (i = 0; i < h0; i++) for (j = 0; j < w0; j++) { 
				iBuf[i + j*h0] = ((iiType)(x0[j + i*w0]));
			} 
			break;
		case mxUINT32_CLASS:  
			uiBuf = (unsigned iiType*) mxGetData(mx);
			for (i = 0; i < h0; i++) for (j = 0; j < w0; j++) { 
				uiBuf[i + j*h0] = ((unsigned iiType)(x0[j + i*w0]));
			} 
			break;
		case mxUINT16_CLASS:  
			usiBuf = (unsigned short iiType*) mxGetData(mx);
			for (i = 0; i < h0; i++) for (j = 0; j < w0; j++) { 
				usiBuf[i + j*h0] = ((unsigned short iiType)(x0[j + i*w0]));
			} 
			break;

		default:
			mexPrintf("IMatrixImpl::toMXArray(): data type unsupported! \n");

	};
}

#endif

//C:\programs\matlab2007b\simulink\include\simstruc.h
//#include "C://programs//matlab2007b//simulink//include//simstruc.h"


//#ifdef MATLAB_MEX_FILE
#ifdef   SIMULINK
void IMatrixImpl::portInit(SimStruct *S, iiType port) {
	iiType  dims =  ssGetInputPortNumDimensions(S, port);
	//   check port dimensions: only 1D and 2D allowed
	if ( (dims < 1) || (dims > 2)) {  
		xm_printf("IMatrixImpl::portInit() error (1)  ( (dims < 1) || (dims > 2))  \n ");
		return;
	}
	iiType width = ssGetInputPortWidth(S, port);
	if (width < 1) {
		xm_printf("IMatrixImpl::portInit() error (2) (width < 1)  \n ");
		return;
	}
	iiType i, j;
	const XMType* src = ssGetInputPortRealSignal(S,port);
	if (dims == 1) {   //    vector
		if (((h0 == width) && (w0 == 1)) || ((w0 == width) && (h0 == 1))) {
			memcpy(x0, src, xsize);
		}  else {
			xm_printf("IMatrixImpl::portInit() error (4)  \n ");
			return;
		}
	} 
	if (dims == 2)  {  //  2D matrix
		int_T* wh = ssGetInputPortDimensions(S,  port);
		iiType h = wh[0];
		iiType w = wh[1];
		if ((h == h0) && (w == w0)) {
			//  remember that matlab likes Fortran very much:
			for (j = 0; j < w; j++)    for (i = 0; i < h; i++) {
				e1(i, j) = src[i+j*h];
			}
		}  else {
			//xm_printf("IMatrixImpl::portInit() error (5);   h0=%d  w0=%d", h0, w0);
			xm_printf("IMatrixImpl::portInit() error (5); \n");
			return;
		}
	}
	
}

void	 IMatrixImpl::output(SimStruct *S, int_T port) {
	iiType dims = ssGetOutputPortNumDimensions(S, port);
	//   check port dimensions: only 1D and 2D allowed:
	if ( (dims < 1) || (dims > 2) ) {
		xm_printf("IMatrixImpl::output() error 1  \n");
		return;
	}
	iiType width = ssGetOutputPortWidth(S, port);
	iiType i, j;
	XMType* dst = ssGetOutputPortRealSignal(S,port);
	if (dims == 1) {
		if (((w0 == 1) && (h0 == width)) || ((w0 == width) && (h0 == 1))) {
			memcpy(dst, x0, xsize);
		}  else {
			xm_printf("IMatrixImpl::output() error 2  \n  ");
			return;
		}
	}
	if (dims == 2) {
		int_T* wh = ssGetOutputPortDimensions(S,  port);
		iiType h = wh[0];
		iiType w = wh[1];
		if ((w0 != w) || (h0 != h)) {
			xm_printf("IMatrixImpl::output()  (error 3) port dims: h = %d  w = %d; my dims:  w0=%d  h0=%d\n", h, w, w0, h0);
#ifdef _DEBUG 
			xm_printf("matrix name = %s\n", name);
#endif
			return;
		}
		for (i = 0; i < h; i++) for (j = 0; j < w; j++) {
			dst[i + j*h] = e1(i, j);	
		}
	}
}
#endif



