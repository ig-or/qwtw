
/**

	3D line data handling.
	

	\file qfpath.cpp
	\author   Igor Sandler
	\date    Jul 2009
	\version 1.0
	
*/



#include "qfpath.h"
#include <math.h>

 QFTriple qfcross(const QFTriple& v1, const QFTriple& v2) {
	QFTriple q;
	q.x[0] = v1.x[1]*v2.x[2] - v1.x[2]*v2.x[1];
	q.x[1] = v1.x[2]*v2.x[0] - v1.x[0]*v2.x[2];
	q.x[2] = v1.x[0]*v2.x[1] - v1.x[1]*v2.x[0];
	return q;
}


QFPath::QFPath() {
	counter1 = 0;
	counter2 = 0;
	N = 0;
	maxColor = 0.0; minColor = 1e8; deltaColor = 1.;
}

static	const double color_change = 0.24;

int QFPath::work1(double* x, int N_, double res_, double* r, double* c) {
	N = N_;
	res = res_;
	if ((x == 0) || (N < 2)) return 0;
	int i;
	double res2;
	for (i = 0; i < 3; i++) {
		bounds[0][i] = x[0 + i*N];
		bounds[1][i] = x[0 + i*N];
	}
	length = 0.;
	counter1 = 1;
	int index = 0, j, k;
	double	dtmp;
	res2 = res * res;

	// =========  calculate bounds and length  =====
	k = 0;
	for (i = 1; i < N; i++) {
		dtmp = 0.;
		for (j = 0; j < 3; j++) {
			dtmp += (x[i + j*N] - x[k + j*N])*(x[i + j*N] - x[k + j*N]);

			if (bounds[0][j] > x[i + j*N]) bounds[0][j] = x[i + j*N];
			if (bounds[1][j] < x[i + j*N]) bounds[1][j] = x[i + j*N];
		}
		if (c != 0) {
			if (c[i] > maxColor) maxColor = c[i];
			if (c[i] < minColor) minColor = c[i];
		}
		if (dtmp > res2) {
			length += sqrt(dtmp);
			k = i;
		}
	}
	if (c != 0) deltaColor = (maxColor - minColor) * color_change;

	dtmp = 0.;
	for (j = 0; j < 3; j++) {
		dtmp += (bounds[1][j] - bounds[0][j]) * (bounds[1][j] - bounds[0][j]);
	}
	//res = sqrt(dtmp) / 256.;
	//res2 = res * res; 

	data1.clear();
	data1.reserve(length / res * 1.1);
	

	//QFTriple	ftmp;
	QFData		qtmp;
	//double		lastColor = 0.8;
	for (j = 0; j < 3; j++) qtmp.p[j] = x[0 + j*N];
	qtmp.i = 0;  // qtmp.p = ftmp;
	if (r != 0) qtmp.r = r[0]; else qtmp.r = 1.0; 
	if (c != 0) { qtmp.c = (c[0] - minColor) / (maxColor - minColor); } else qtmp.c = 0.8;
	data1.append(qtmp);

	for (i = 1; i < N; i++) {
		dtmp = 0.;
		for (j = 0; j < 3; j++) {
			dtmp += (x[i + j*N] - qtmp.p[j])*(x[i + j*N] - qtmp.p[j]);
		}
		if ((dtmp >= res2) || 
				((c != 0) && (fabs(((c[i] - minColor) / (maxColor - minColor)) - 
				                            qtmp.c) > color_change))) {

			for (j = 0; j < 3; j++) qtmp.p[j] = x[i + j*N];
			qtmp.i = i;  // qtmp.p = ftmp;
			if (r != 0) qtmp.r = r[i]; else qtmp.r = 1.0; 
			if (c != 0) { qtmp.c = (c[i] - minColor) / (maxColor - minColor);  } else qtmp.c = 0.8;
			counter1 += 1;
			data1.append(qtmp);
		}
	}
	return counter1;
}

//bool QFPath::dxTest(const QFTriple& x1, const QFTriple& x2, const QFTriple& x) {
	//
	//return true;  // double V
//}

int QFPath::work2() {
	if (counter1 < 3) return 0;

	counter2 = 1;
	data2.clear();
	//data2.resize(counter1 >> 2);
	data2.reserve(counter1 >> 2);
	data2.append(data1[0]);

	int i, j, k;
	i = 0;
	bool far;
	const double	dRes = 180.;
	QFTriple v, w, ftmp;
	QFData		qtmp;
	double vw, distance;

	while (i < (counter1 - 2)) {
		k = i + 2; 
		while (k < (counter1)) {
			far = false;

			//  check distance from k to i:
			v = data1[k].p - data1[i].p;
			vw = v.length();

			if (vw > dRes * res) {
				far = true;
				//break;
			}
			if (fabs(data1[i].c - data1[k-1].c) > color_change) {
				far = true;
			}

			for (j = i + 1; ((!far) && (j < k)); j++) {
				// http://algolist.manual.ru/maths/geom/distance/pointline.php
				w = data1[j].p - data1[i].p;
				ftmp = qfcross(v, w);
				distance = ftmp.length() / vw;
				if (distance >= res) {
					far = true;
					break;
				}
			}
			if (far) {  //  save previous point:
				data2.append(data1[k-1]);
				counter2 += 1;
				i = k-1;
				break;
			} else {  //  check next point:
				k += 1;
			}
		}  // while (k < (counter1))
		if (k >= (counter1 - 1)) {
			break;
		}
	}
	//  add last point:
	data2.append(data1[counter1 - 1]);
	counter2 += 1;
	return counter2;
}

