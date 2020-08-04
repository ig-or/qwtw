/**  The matrix hangling library.

	\file xmatrix2.cpp
	\author   Igor Sandler
	\date    nov 2006
	\version 2.0 (2013)
	
*/

#include "xmatrix2.h"
#include <stdlib.h>

const DCM	eye33 = eye<3, 3>();

/*
DCM	euler2dcm_strange(const Vector3& a) {
	DCM m("DCM");
	XMType ca = cos(a(0)), sa = sin(a(0));
	XMType cb = cos(a(1)), sb = sin(a(1));
	XMType cg = cos(a(2)), sg = sin(a(2));

	m(0, 0) = ca * cb; 
	m(1, 0) = sb; 	
	m(2, 0) = -sa * cb;
	m(0, 1) = -ca*sb*cg + sa*sg; 
	m(1, 1) = cb*cg; 
	m(2, 1) = ca*sg + sa*sb*cg;
	m(0, 2) =  ca*sb*sg + sa*cg;
	m(1, 2) = -cb*sg;
	m(2, 2) = ca*cg - sa*sb*sg;

	return m;
}
*/

DCM euler2dcm(const Vector3& eu) {

	DCM m("DCM");
	XMType cz = cos(eu(0)), sz = sin(eu(0));
	XMType cy = cos(eu(1)), sy = sin(eu(1));
	XMType cx = cos(eu(2)), sx = sin(eu(2));

	
	m(0, 0) = cy * cz;
	m(0, 1) = cy*sz;
	m(0, 2) = -sy;

	m(1, 0) = sy*sx*cz - cx*sz;
	m(1, 1) = sy*sx*sz + cz*cx;
	m(1, 2) = sx*cy;

	m(2, 0) = cx*sy*cz + sz*sx;
	m(2, 1) = cx*sy*sz - sx*cz;
	m(2, 2) = cy*cx;

/*  THIS IS NOT CORRECT
	m(0, 0) = c2 * c1;
	m(1, 0) = c2*s1;
	m(2, 0) = -s2;
	m(0, 1) = s3*s2*c1 - c3*s1;
	m(1, 1) = s3*s2*s1 + c3*c1;
	m(2, 1) = s3*c2;
	m(0, 2) = c3*s2*c1 + s3*s1;
	m(1, 2) = c3*s2*s1 - s3*c1;
	m(2, 2) = c3*c2;
*/
	return m;
}

DCM	euler2dcm_small(const Vector3& a) {
	DCM m("DCM small");
	XMType yaw =   a(0);
	XMType pitch = a(1);
	XMType roll =  a(2);
	m(0, 0) = ONE;		m(1, 0) = pitch; 	m(2, 0) = -yaw;
	m(0, 1) = -pitch;	m(1, 1) = ONE;		m(2, 1) = roll;
	m(0, 2) =  yaw;		m(1, 2) = -roll;	m(2, 2) = ONE;

	return m;
}

DCM euler2dcm_YZ(XMType b, XMType g) {
	DCM dcm;
	XMType cb = cos(b); 
	XMType sb = sin(b); 
	XMType cg = cos(g); 
	XMType sg = sin(g); 

	//  create matrix of a small rotation 
	//  around Y (first rotation) and Z (second rotation):
	dcm(0, 0) = cg*cb;		dcm(0, 1) = sg;		dcm(0, 2) = -cg*sb; 
	dcm(1, 0) = -sg*cb;		dcm(1, 1) = cg;		dcm(1, 2) = sg*sb; 
	dcm(2, 0) = sb;			dcm(2, 1) = ZERO;	dcm(2, 2) = cb; 
	return dcm;
}

Vector3	cross(const Vector3& m1, const Vector3& m2) {
	Vector3 v;
	v(0) =  m1(1)*m2(2) - m2(1)*m1(2);
	v(1) =  m2(0)*m1(2) - m1(0)*m2(2);
	v(2) =  m1(0)*m2(1) - m2(0)*m1(1);
	return v;
}

M33	crossform(const Vector3& v) {
	M33 ret("ret");
	ret(0, 0) =  ZERO;		ret(0, 1) =  v(2);		ret(0, 2) = -v(1);
	ret(1, 0) = -v(2);		ret(1, 1) =  ZERO;		ret(1, 2) =  v(0);
	ret(2, 0) =  v(1);		ret(2, 1) = -v(0);		ret(2, 2) =  ZERO;

	return ret;
}
M33	crossform2(const Vector3& v) {
	M33 ret =  crossform(v);
	ret.dmul(-ONE);
	return ret;
}

Vector3	dcm2euler(const DCM& dcm) {
	Vector3 ret("euler");
	 //  this old variant I do not understand.
/*	XMType sb = dcm(1, 0);
	ret[1] = asin(sb); // pitch
	
	ret[2] = atan2(-dcm(1, 2), dcm(1, 1)); //   roll
	ret[0] = atan2(-dcm(2, 0), dcm(0, 0)); //	heading
*/

	
	ret.x[0] = atan2(dcm(0,1), dcm(0, 0));
	ret.x[1] = asin(-dcm(0, 2));
	ret.x[2] = atan2(dcm(1, 2), dcm(2, 2));
	
/*
	//    let's try another way:
	ret(0) = atan2(dcm(1,0), dcm(0, 0));
	ret(1) = asin(-dcm(2, 0));
	ret(2) = atan2(dcm(2, 1), dcm(2, 2));
*/
	return ret;
}


Qua euler2qua(const Vector3& eu) {
	Qua q;
	XMType ca = cos(HALF*eu(0)), sa = sin(HALF*eu(0));
	XMType cb = cos(HALF*eu(1)), sb = sin(HALF*eu(1));
	XMType cg = cos(HALF*eu(2)), sg = sin(HALF*eu(2));

	// ================== ZYX:============
      q.x[0] = ca*cb*cg + sa*sb*sg;
      q.x[1] = ca*cb*sg - sa*sb*cg;
      q.x[2] = ca*sb*cg + sa*cb*sg;
      q.x[3] = sa*cb*cg - ca*sb*sg;
	 
	return q;
}

M33 MWL(const V3& wr, const V3& l) {
	M33 Mwl;
	Mwl(0, 0) = 0.0;				Mwl(0, 1) = wr[1] * l[2];		Mwl(0, 2) = -wr[2] * l[1];
	Mwl(1, 0) = -wr[0] * l[2];		Mwl(1, 1) = 0.0;				Mwl(1, 2) = wr[2] * l[0];
	Mwl(2, 0) = wr[0] * l[1];		Mwl(2, 1) = -wr[1] * l[0];		Mwl(2, 2) = 0.0;
	return Mwl;
}



// ################################################################3

/*		INIT  empty quaternion*/
Qua::Qua(const char* _name): IMatrix<4, 1>(_name)  {
	x[0] = ONE;
}

/*		INIT   quaternion  from matrix  4x1*/
Qua::Qua (const IMatrix<4, 1>& m, const char* _name): IMatrix<4, 1>(m, _name) {  }

/* init from rotation vector  */
Qua::Qua (const Vector3& v,  const char* _name): IMatrix<4, 1>(_name)  {
	from_rv(v);
/*	XMType df = v.norma2();
	if (df <= DELTA_ZERO_1) {
		x0[0] = ONE;
	}  else {
		df = sqrt(df);
		XMType tmp = sin(df / 2) / df;
		x0[0] = cos(df / 2);
		x0[1] = tmp*v(0);
		x0[2] = tmp*v(1);
		x0[3] = tmp*v(2);
	}
	*/
}

/*   init quaternion from DCM  */
Qua::Qua(const DCM& m,   const char* _name): IMatrix<4, 1>(_name)  {
	dcm2qua_(x, m.x);
}

/*	init quaternion from another one*/
Qua::Qua (const Qua& q, const char* _name): IMatrix<4, 1>(_name)   {
	memcpy(x, q.x, 4*XMTSize);
}

// 		assign matrix to quaternion 
/*
Qua& Qua::operator=(const IMatrix<4, 1>& m) {
	memcpy(x, m.x, 4*XMTSize);
	return *this;
}
*/

void Qua::setYaw(XMType yaw) { 
	yaw = yaw;
	/*  not correct way!
	Vector3	eu = euler();
	eu[0] = yaw;
	Qua q;

	q = euler2qua(eu);
	x[0] = q.x[0]; 
	x[1] = q.x[1];
	x[2] = q.x[2];
	x[3] = q.x[3];
	*/
}

XMType Qua::yaw() {
	Vector3	eu = euler();
	return eu(0);
}

Vector3 Qua::euler() const {
	Vector3 eu("eu");

	XMType qm = sqrt(x[0]*x[0] + x[1]*x[1] + x[2]*x[2] + x[3]*x[3]);
	XMType q[4];
	q[0] = x[0] / qm; 
	q[1] = x[1] / qm; 
	q[2] = x[2] / qm; 
	q[3] = x[3] / qm; 

	//  ================ ZYX ==================
	XMType r11 = TWO*(q[1]*q[2] + q[0]*q[3]);
	XMType r12 = q[0]*q[0] + q[1]*q[1] - q[2]*q[2] - q[3]*q[3];
	XMType r21 = (-TWO)*(q[1]*q[3] - q[0]*q[2]);
	XMType r31 = TWO*(q[2]*q[3] + q[0]*q[1]);
	XMType r32 = q[0]*q[0] - q[1]*q[1] - q[2]*q[2] + q[3]*q[3];

	//mxassert((r21 <= ONE) && (r21 >= -ONE), "quat not normilized?");

	if(r21 >= ONE) {
	    eu.x[1] = pii / TWO;
	} else if(r21 <= (-ONE)) {
		eu.x[1] = -pii / TWO;
	} else {
		eu.x[1] = asin(r21);
	}

	eu.x[0] = atan2(r11, r12);
	eu.x[2] = atan2(r31, r32);
	

/*	//  second version: (http://www.pololu.com/file/0J588/AN-1006-UnderstandingQuaternions.pdf)
	XMType r11 = TWO*(q[0]*q[1] + q[2]*q[3]);
	XMType r12 = q[0]*q[0] - q[1]*q[1] - q[2]*q[2] + q[3]*q[3];
	XMType r21 = (TWO)*(q[1]*q[3] - q[0]*q[2]);
	XMType r31 = TWO*(q[0]*q[3] + q[1]*q[2]);
	XMType r32 = q[0]*q[0] + q[1]*q[1] - q[2]*q[2] - q[3]*q[3];

	eu[0] = atan2(r11, r12);
	eu[1] = -asin(r21);
	eu[2] = atan2(r31, r32);
*/
	return eu;
}


//Qua operator*(XMType d, const Qua& m) {
//	return m * d;
//}

void slerp510(const Qua& from, const Qua& to, XMType t, Qua& res) {
	XMType cosOmega = from[0]*to[0] + from[1]*to[1] + from[2]*to[2] + from[3]*to[3];
	XMType toTmp[4];
	XMType scale[2];

	// adjust signs if needed, i.e. if we aren't in same region
	if (cosOmega < ZERO) {
		cosOmega = -cosOmega;
		for (int i=0; i < 4; i++)
			toTmp[i] = -to[i];
	} else {
		for (int i=0; i < 4; i++)
			toTmp[i] = to[i];
	}

	// compute coefficients
	if ((ONE - cosOmega) > 0.000001) { // do slerp if difference is big enough
		mxassert((cosOmega > -1.) && (cosOmega < 1.), "");

		XMType omega = acos(cosOmega);
		XMType sinOmega = sin(omega);
		scale[0] = sin((ONE - t) * omega) / sinOmega;
		scale[1] = sin(t * omega) / sinOmega;
	} else { // do linear interpolation if too close
		scale[0] = ONE - t;
		scale[1] = t;
	}

	// compute result
	for (int i=0; i < 4; i++) {
		res.x[i] = scale[0]*from[i] + scale[1]*toTmp[i];
	}
	res.normalize();
}

Qua operator*(const Qua& q1, const Qua& q2) {
	Qua ret;
	ret.x[0] = q1.x[0] * q2.x[0] - q1.x[1] * q2.x[1] - q1.x[2] * q2.x[2] - q1.x[3] * q2.x[3];
	ret.x[1] = q1.x[0] * q2.x[1] + q1.x[1] * q2.x[0] + q1.x[2] * q2.x[3] - q1.x[3] * q2.x[2];
	ret.x[2] = q1.x[0] * q2.x[2] - q1.x[1] * q2.x[3] + q1.x[2] * q2.x[0] + q1.x[3] * q2.x[1];
	ret.x[3] = q1.x[0] * q2.x[3] + q1.x[1] * q2.x[2] - q1.x[2] * q2.x[1] + q1.x[3] * q2.x[0];

	return ret;
}


Qua dcm2qua(const DCM& m) {
	Qua q;
//	IMatrixImpl::dcm2qua_(q.x, m.x);
	
	XMType trace = m.trace(); 
	if(trace > 0) {
		XMType s = 0.5 / sqrt(trace + 1.0);
		q(0) = 0.25 / s;
		q(1) = (m(2,1) - m(1,2)) * s;
		q(2) = (m(0,2) - m(2,0)) * s;
		q(3) = (m(1,0) - m(0,1)) * s;
	} else {
		if(m(0, 0) > m(1, 1) && m(0, 0) > m(2, 2)) {
			XMType s = 2.0 * sqrt(1.0 + m(0, 0) - m(1, 1) - m(2, 2));
			q(0) = (m(2, 1) - m(1, 2)) / s;
			q(1) = 0.25 * s;
			q(2) = (m(0, 1) + m(1, 0)) / s;
			q(3) = (m(0, 2) + m(2, 0)) / s;
		} else if(m(1, 1) > m(2, 2)) {
			XMType s = 2.0 * sqrt(1.0 + m(1, 1) - m(0, 0) - m(2, 2));
			q(0) = (m(0, 2) - m(2, 0)) / s;
			q(1) = (m(0, 1) + m(1, 0)) / s;
			q(2) = 0.25 * s;
			q(3) = (m(1, 2) + m(2, 1)) / s;
		} else {
			XMType s = 2.0 * sqrt(1.0 + m(2, 2) - m(0, 0) - m(1, 1));
			q(0) = (m(1,0) - m(0,1)) / s;
			q(1) = (m(0,2) + m(2,0)) / s;
			q(2) = (m(1,2) + m(2,1)) / s;
			q(3) = 0.25 * s;
		}
	}
	if(q[0] < ZERO) {
		q.dmul(-ONE);
	}
	return q.conj();  //  TODO: fix this
	
//	return q;
}

Qua rotationFromTwoVectors(const V3& v1i, const V3& v1e, const V3& v2i, const V3& v2e) {
	/// clever algorithm from Danila Milutin below:
	Vector3 v3i = cross(v1i, v2i);  //    create third vector in 'i'
	Vector3 v3e = cross(v1e, v2e);	//    create third vector in 'e'
	M33 Vi; // 'fame' created from three vectors in 'i'
	Vi.insert2(v1i, 0, 0);
	Vi.insert2(v2i, 0, 1);
	Vi.insert2(v3i, 0, 2);
	M33 Ve; // 'fame' created from three vectors in 'e'
	Ve.insert2(v1e, 0, 0);
	Ve.insert2(v2e, 0, 1);
	Ve.insert2(v3e, 0, 2);

	Vi.invSelf(); 
	//M33 Q1 = Vimu * Venu;  this is not correct
	M33 Q1 = Ve * Vi; //    but, this is not a 'DCM' yet


	M33 Q, R;
	Qua q;
	bool ok = Q1.qr_big(Q, R); // FIXME: create not so complex way for orientation determination
	if(!ok) {
		mxassert(false, " rotationFromTwoVectors() faild ");
		return q;
	}
	//xm_printf("\talign: R= [%.3f  %.3f  %.3f  %.3f  %.3f  %.3f  %.3f  %.3f  %.3f]\n",
	//	R(0), R(1), R(2), R(3), R(4), R(5), R(6), R(7), R(8));

	q = dcm2qua(Q); // TODO: this function may be not OK. 
	return q;
}


// ########################################################################################
// ########################################################################################
// ########################################################################################
// ########################################################################################

int roundX(XMType x) {
	XMType	x1 = (floor(x));
	if ((x - x1) > HALF) { 
		return ceil(x);
	} else {
		return x1;
	}
}

long long roundXL(long double x) {
	long double 	x1 = (floor(x));
	if ((x - x1) > HALF) { 
		return ceil(x);
	} else {
		return x1;
	}
}
/*
long long roundXL(XMType x) {
	XMType 	x1 = (floor(x));
	if ((x - x1) > HALF) {
		return ceil(x);
	} else {
		return x1;
	}
}
*/

/**
	get gravitation from latitude and height
  */
XMType normalgravity(XMType latitude, XMType he)  {
	
	const XMType a1 = 9.7803267715;
	const XMType a2 = 0.0052790414;
	const XMType a3 = 0.0000232718;
	const XMType a4 = -0.000003087691089;
	const XMType a5 = 0.000000004397731;
	const XMType a6 = 0.000000000000721;

	XMType s2 = sin(latitude)*sin(latitude);
	XMType s4 = s2 * s2;
	XMType ng = a1 * (ONE + a2*s2 + a3*s4) + (a4 + a5*s2)*he + a6 * he * he;

	return ng;
};

XMType paraint(XMType dt, XMType  y1, XMType  y2, XMType  y3) {
	// 1. 
     XMType dt2 = dt*dt;
	XMType c = y1;
	XMType b = (FORE * y2 - THREE*y1 - y3) / (TWO*dt);
	XMType a = (y3 - y1 - TWO*b*dt) / (FORE*dt2);
	// 2.
	XMType s = (SEVEN / THREE) * a * dt*dt2 + (THREE / TWO) * b*dt2 + c*dt;
	return s;
}
XMType linInterp(long long x1, long long  x2, long long  xm, XMType y1, XMType y2) {
    long long dx = x2 - x1, xx = xm - x1;
    XMType y = y1 + (y2 - y1) * ((XMType)(xx)) / ((XMType)(dx));
    return y;
}

XMType linInterp(XMType	x1, XMType x2, XMType xm, XMType y1, XMType y2) {
	XMType y = y1 + ((y2 - y1) * (xm - x1) / (x2 - x1));
	return y;
}

Vector3 linInterp(const Vector3& v1, const Vector3& v2, XMType t) {
	Vector3 ret;
	for(int i = 0; i < 3; i++) {
		ret.x[i] = (1. - t) * v1.x[i] + t*v2.x[i];
	}
	
	return ret;
}

XMType lineDist(XMType* ab, XMType x, XMType y) {
	XMType A = ab[0];
	XMType C = ab[1];

	XMType d = fabs(A*x -y + C) / sqrt(A*A+ONE);
	return d;
}

void	linear_appr(XMType*	x, XMType*	y, int N, XMType* ab) {
	int i;
	XMType sx = ZERO, sx2 = ZERO, sxy = ZERO, sy = ZERO;

	for (i = 0; i < N; i++) {
		sx2 += x[i] * x[i];
		sx += x[i];
		sxy += x[i] * y[i];
		sy += y[i];
	}
	ab[0] = (sxy*N - sx*sy) / (N*sx2 - sx*sx);
	ab[1] = (sy*sx2 -sx*sxy) / (N*sx2 - sx*sx);
}

void 	linear_appr2(XMType*	x, XMType*	y, int N, XMType d, XMType* ab) {
	int i;
	if (N < 2) {
		xm_printf("linear_appr2 failed\n");
		return;
	}
	XMType sx = ZERO, sy = ZERO, sxx = ZERO, syy = ZERO, sxy = ZERO, n, fx, fy;
	n = static_cast<XMType>(N);
	for (i = 0; i < N; i++) {
		sx += x[i];  sy += y[i];
	}
	sx /= n;    sy /= n;
	for (i = 0; i < N; i++) {
		fx = x[i] - sx;
		fy = y[i] - sy;
		sxx += fx * fx;
		syy += fy * fy;
		sxy += fx * fy;
	}
	sxx /= (n - ONE);
	syy /= (n - ONE);
	sxy /= (n - ONE);

	ab[0] = (syy - d * sxx + sqrt(sqr(syy - d*sxx) + FORE * d * sxy*sxy)) / (TWO * sxy);
	ab[1] = sy - ab[0] * sx;
}

void     parabola_appr(XMType*	x, XMType*	y, int N, XMType*	abc) {
	XMType x4 = ZERO, x3 = ZERO, x2 = ZERO, x1 = ZERO, yx2 = ZERO, yx1 = ZERO, y1 = ZERO;
	int i;
	XMType xx;
	for (i = 0; i < N; i++) {
		y1 += y[i];
		xx = x[i];	//  x
		x1 += xx;
		yx1 += y[i] * xx;
		xx *= x[i];	// x^2
		yx2 += y[i] * xx;
		x2 += xx;
		xx *= x[i];   //   x^3
		x3 += xx;
		xx *= x[i];    //    x^4
		x4 += xx;
	}
	XMType ab, at, bt, ct;
	XMType n = (XMType)(N);

	ab = n*x2*x4 - x2*x2*x2 - n*x3*x3 - x1*x1*x4 + 2.*x3*x1*x2;
	if (fabs(ab) < DELTA_ZERO_1) {   //    a   line?
		linear_appr(x, y, N, abc);
		abc[2] = abc[1];   abc[1] = abc[0];   abc[0] = ZERO;
	} else {
		at = x3*x1*y1 - n*x3*yx1 + n*x2*yx2 - x2*x2*y1 - x1*x1*yx2 + x1*x2*yx1;
		bt = -yx1*x2*x2 + x2*x1*yx2 + x2*x3*y1 - n*x3*yx2 + n*yx1*x4 - x1*y1*x4;
		ct = x2*x3*yx1 - x2*x2*yx2 + x1*x3*yx2 - x1*yx1*x4 + y1*x2*x4 - y1*x3*x3;
		
		abc[0] = at / ab;
		abc[1] = bt / ab;
		abc[2] = ct / ab;
	}
}

void logSpace( XMType *array, int size, XMType xmin, XMType xmax )
{
	mxassert((xmin > 0) && (xmax > 0) && (size > 0), "logSpace");
    if ( ( xmin <= ZERO ) || ( xmax <= ZERO ) || ( size <= 0 ) )
        return;

    const int imax = size - 1;

    array[0] = xmin;
    array[imax] = xmax;

    const XMType lxmin = log( xmin );
    const XMType lxmax = log( xmax );
    const XMType lstep = ( lxmax - lxmin ) / XMType( imax );

    for ( int i = 1; i < imax; i++ )
        array[i] = ::exp( lxmin + XMType( i ) * lstep );
}

XMType rand1() {
	//const int mr = 0x7fff;
	XMType ret = ((XMType)(rand())) / ((XMType)(RAND_MAX));
	//XMType ret = ((XMType)(rand())) / ((XMType)(mr));
	return ret;
}

XMType rand2() {
	//const int mr = 0x7fff;
	XMType ret = ((XMType)(rand())) / ((XMType)(RAND_MAX)) - 0.5;
	return ret;
}

static int randgauss_flag = 1;
static XMType randgauss_num;
XMType randgauss() {
	if (randgauss_flag == 2) {  
		randgauss_flag = 1;
		return randgauss_num;
	}  else {
		randgauss_flag = 2;
		XMType x1, x2;
		XMType w;
		XMType y1, y2;
		 do {
                 x1 = TWO * rand1() - ONE;
                 x2 = TWO * rand1() - ONE;
                 w = x1 * x1 + x2 * x2;
         } while ( w >= ONE );

         w = sqrt( (-TWO * log( w ) ) / w );
         y1 = x1 * w;
         y2 = x2 * w;
		
		 randgauss_num = y2;
		 return y1;
	}
}
XMType randgauss(XMType sigma) {
	return randgauss() * sigma;
}


#ifdef PC__VERSION

Line3DSimple::Line3DSimple() {
	x = 0; y = 0; z = 0; n = 0; t = 0;
	own = false;  index = 0;
}
Line3DSimple::Line3DSimple(int n_, XMType* x_, XMType* y_,
	XMType* z_, XMType* t_, bool own_) {

	n = n_;
	own = own_;
	index = 0;

	if (own) {
		x = new XMType[n];
		y = new XMType[n];
		z = new XMType[n];
		t = new XMType[n];

		memcpy(x_, x, n * sizeof(XMType));
		memcpy(y_, y, n * sizeof(XMType));
		memcpy(z_, z, n * sizeof(XMType));
		memcpy(t_, t, n * sizeof(XMType));
	} else {
		x = x_; y = y_; z = z_; t = t_;
	}
}

Line3DSimple::Line3DSimple(int n_) {
	n = n_;
	x = new XMType[n];
	y = new XMType[n];
	z = new XMType[n];
	t = new XMType[n];
	own = true;
	index = 0;
}

void Line3DSimple::feedPoint(XMType* p, XMType time) {
	if (index >= n) return;
	x[index] = p[0];
	y[index] = p[1];
	z[index] = p[2];
	t[index] = time;
	index++;
}

Line3DSimple::~Line3DSimple() {
	if ((n != 0) && own) {
		delete[] x;  delete[] y; delete[] z; delete[] t;
		n = 0;
	}
	index = 0;
}


Line3DSimple* reduceLine(const Line3DSimple& line, XMType maxDist) {
	//XMType maxDist = 0.1 * 0.1;
	//XMType maxDist = 1.0 * 1.0;
	int* ind = new int[line.n];
	int i, j, index = 0;
	ind[index] = 0;
	XMType dx;
	
	for (i = 1; i < (line.n-1); i++) {
		dx = sqr(line.x[i] - line.x[ind[index]]) + 
			sqr(line.y[i] - line.y[ind[index]]) +
			sqr(line.z[i] - line.z[ind[index]]);
		if (dx < maxDist) {
			continue;
		}
		j = i - 1;
		if (j == ind[index]) j = i;
		index++;
		ind[index] = j;
	}
	index++;
	ind[index] = line.n-1;

	//xm_printf("@\t - reduceLine: %d -> %d\n", line.n, index+1);
	Line3DSimple* ret = new Line3DSimple(index + 1);

	for (i = 0; i < ret->n; i++) {
		ret->x[i] = line.x[ind[i]];
		ret->y[i] = line.y[ind[i]];
		ret->z[i] = line.z[ind[i]];
		ret->t[i] = line.t[ind[i]];
	}

	delete[] ind;
	return ret;
}

int reduceLine2(XMType* x, XMType* y, XMType* z, XMType* t, int& n) {





	return 0;
}


DMatrix::DMatrix() {
	x = 0; ww = 0; hh = 0;
}

DMatrix::~DMatrix() {
	clear();
}

void DMatrix::init(int h_, int w_) {
	clear();
	hh = h_;  ww = w_;
	x = new XMType[ww*hh];
	memset(x, 0, ww*hh*sizeof(XMType));
}

void DMatrix::clear() {
	if (x != 0) {
		delete[] x;
		x = 0;
	}
	hh = 0; ww = 0;
}


void DMatrixPool::addMatrix(DMatrix* m, int h, int w) {
	if((h == 0) || (w == 0)) {
		return;
	}
    std::map<DMatrix*, DMatrix*>::iterator it = dMatrixMap.find(m);
    if(it == dMatrixMap.end()) {
	   dMatrixMap.insert(std::make_pair(m, m));
    }
	m->init(h, w);
}
void DMatrixPool::clearMatrices() {
    std::map<DMatrix*, DMatrix*>::iterator it;
    for(it = dMatrixMap.begin(); it != dMatrixMap.end(); it++) {
	   it->second->clear();
	}
    dMatrixMap.clear();
}



#endif
// ########################################################################################
// ########################################################################################
// ########################################################################################
// ########################################################################################
#ifdef MATLAB_MEX_FILE
XMType MWSVar::errorValue = 11.0;

MWSVar::MWSVar(int hh, int ww, const char* name1)  {
	v = 0;  count = 0;
	init(hh, ww, name1);
}
void MWSVar::init(int hh, int ww, const char* name1)  {
	clear();
	strncpy(name, name1, 63);
	h = hh;  w = ww;
	//mptr = mxCreateDoubleMatrix(h, w, mxREAL);
	//v = mxGetPr(mptr);
	count = 0;

	v = new XMType[h*w];
	if (v == NULL) {
		xm_printf("out of memory error. MWSVar::init: v == 0;  \n");
	}
}
void MWSVar::init(int hh, int ww, const char* prefix, const char* name1, int number) {
	char wtmp[36];
	snprintf(wtmp, 32, "%s_%s%d", prefix, name1, number);
	init(hh, ww, wtmp);
}
MWSVar::MWSVar(int hh, int ww, const char* prefix, const char* name1, int number) {
	init(hh, ww, prefix, name1, number);
}

MWSVar::~MWSVar() {
	clear();
}

void MWSVar::clear() {
	if (v != 0) {
		delete[] v;
		v = 0;
	}
	count = 0;
}

void MWSVar::add(const Vector3& a) {
	if ((h != 3) || (count >= w)) {
		return;
	}
	v[0 + count * h] = a(0);
	v[1 + count * h] = a(1);
	v[2 + count * h] = a(2);
	count++;
}

void MWSVar::stay() {
	if ((h != 3) || (count < 1)) {
		return;
	}
	Vector3 tmp;
	tmp[0] = v[0 + (count-1)*h];
	tmp[1] = v[1 + (count-1)*h];
	tmp[2] = v[2 + (count-1)*h];
	add(tmp);

}

XMType& MWSVar::operator()(int i, int j) {
#ifdef _DEBUG 
	if ((i < 0) || (i >= h) || (j < 0) || (j >= w)) {
		xm_printf("MWSVar: index out of bounds \n");
		return MWSVar::errorValue;
	}
#endif
	return v[i + j * h];
}

void MWSVar::save() {
	//  if 'count' was used, then use count;  else use 'w'
	int w3 = (count == 0) ? w : count;
	mptr = mxCreateDoubleMatrix(h, w3, mxREAL);
	XMType* v1 = mxGetPr(mptr);
	memcpy(v1, v, h*w3*sizeof(XMType));
	int status = mexPutVariable("base", name, mptr);
	if (status == 1) {  //  not OK
		mexPrintf("MWSVar::save(): mexPutVariable faild. \n");
	}
	mxDestroyArray(mptr);
}


#endif








