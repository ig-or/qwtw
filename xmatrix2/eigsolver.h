
/*

C++ module 'eig3' by Connelly Barnes
------------------------------------

License: public domain.

The source files in this directory have been copied from the public domain
Java matrix library JAMA.  The derived source code is in the public domain
as well.

Usage:

// Symmetric matrix A => eigenvectors in columns of V, corresponding
//eigenvalues in d. 

*/

#ifndef EIG_SOLVER_H_FILE
#define EIG_SOLVER_H_FILE

#include "xmatrix2.h"

#ifdef MAX_123
#undef MAX_123
#endif

#define MAX_123(a, b) ((a)>(b)?(a):(b))

/* Eigen-decomposition for symmetric 4х4 real matrices.
Public domain, copied from the public domain Java library JAMA. */

/** Symmetric matrix A => eigenvectors in columns of V, corresponding
eigenvalues in d. */
void eigen_decomposition(XMType A[4][4], XMType V[4][4], XMType d[4]);

XMType hypot2(XMType x, XMType y);


/** Symmetric Householder reduction to tridiagonal form.
*/
template<int n>  void eigen_tred2(IMatrix<n, n>& V, IMatrix<n, 1>& d, IMatrix<n, 1>& e) {

	//  This is derived from the Algol procedures tred2 by
	//  Bowdler, Martin, Reinsch, and Wilkinson, Handbook for
	//  Auto. Comp., Vol.ii-Linear Algebra, and the corresponding
	//  Fortran subroutine in EISPACK.

	for (int j = 0; j < n; j++) {
		d[j] = V(n - 1,j);
	}

	// Householder reduction to tridiagonal form.

	for (int i = n - 1; i > 0; i--) {

		// Scale to avoid under/overflow.

		XMType scale = 0.0;
		XMType h = 0.0;
		for (int k = 0; k < i; k++) {
			scale = scale + fabs(d[k]);
		}
		if (scale == 0.0) {
			e[i] = d[i - 1];
			for (int j = 0; j < i; j++) {
				d[j] = V(i - 1, j);
				V(i,j) = 0.0;
				V(j,i) = 0.0;
			}
		} else {

			// Generate Householder vector.

			for (int k = 0; k < i; k++) {
				d[k] /= scale;
				h += d[k] * d[k];
			}
			XMType f = d[i - 1];
			XMType g = sqrt(h);
			if (f > 0) {
				g = -g;
			}
			e[i] = scale * g;
			h = h - f * g;
			d[i - 1] = f - g;
			for (int j = 0; j < i; j++) {
				e[j] = 0.0;
			}

			// Apply similarity transformation to remaining columns.

			for (int j = 0; j < i; j++) {
				f = d[j];
				V(j, i) = f;
				g = e[j] + V(j, j) * f;
				for (int k = j + 1; k <= i - 1; k++) {
					g += V(k, j) * d[k];
					e[k] += V(k, j) * f;
				}
				e[j] = g;
			}
			f = 0.0;
			for (int j = 0; j < i; j++) {
				e[j] /= h;
				f += e[j] * d[j];
			}
			XMType hh = f / (h + h);
			for (int j = 0; j < i; j++) {
				e[j] -= hh * d[j];
			}
			for (int j = 0; j < i; j++) {
				f = d[j];
				g = e[j];
				for (int k = j; k <= i - 1; k++) {
					V(k, j) -= (f * e[k] + g * d[k]);
				}
				d[j] = V(i - 1, j);
				V(i, j) = 0.0;
			}
		}
		d[i] = h;
	}

	// Accumulate transformations.

	for (int i = 0; i < n - 1; i++) {
		V(n - 1, i) = V(i, i);
		V(i, i) = 1.0;
		XMType h = d[i + 1];
		if (h != 0.0) {
			for (int k = 0; k <= i; k++) {
				d[k] = V(k, i + 1) / h;
			}
			for (int j = 0; j <= i; j++) {
				XMType g = 0.0;
				for (int k = 0; k <= i; k++) {
					g += V(k, i + 1) * V(k, j);
				}
				for (int k = 0; k <= i; k++) {
					V(k, j) -= g * d[k];
				}
			}
		}
		for (int k = 0; k <= i; k++) {
			V(k, i + 1) = 0.0;
		}
	}
	for (int j = 0; j < n; j++) {
		d[j] = V(n - 1, j);
		V(n - 1, j) = 0.0;
	}
	V(n-1, n-1) = 1.0;
	e[0] = 0.0;
}

/** Symmetric tridiagonal QL algorithm.
*/

template<int n>  void eigen_tql2(IMatrix<n, n>& V, IMatrix<n, 1>& d, IMatrix<n, 1>& e) {

	//  This is derived from the Algol procedures tql2, by
	//  Bowdler, Martin, Reinsch, and Wilkinson, Handbook for
	//  Auto. Comp., Vol.ii-Linear Algebra, and the corresponding
	//  Fortran subroutine in EISPACK.

	for (int i = 1; i < n; i++) {
		e[i - 1] = e[i];
	}
	e[n - 1] = 0.0;

	XMType f = 0.0;
	XMType tst1 = 0.0;
	XMType eps = pow(2.0, -52.0);
	for (int l = 0; l < n; l++) {

		// Find small subdiagonal element

		tst1 = MAX_123(tst1, fabs(d[l]) + fabs(e[l]));
		int m = l;
		while (m < n) {
			if (fabs(e[m]) <= eps*tst1) {
				break;
			}
			m++;
		}

		// If m == l, d[l] is an eigenvalue,
		// otherwise, iterate.

		if (m > l) {
			int iter = 0;
			do {
				iter = iter + 1;  // (Could check iteration count here.)

								  // Compute implicit shift

				XMType g = d[l];
				XMType p = (d[l + 1] - g) / (2.0 * e[l]);
				XMType r = hypot2(p, 1.0);
				if (p < 0) {
					r = -r;
				}
				d[l] = e[l] / (p + r);
				d[l + 1] = e[l] * (p + r);
				XMType dl1 = d[l + 1];
				XMType h = g - d[l];
				for (int i = l + 2; i < n; i++) {
					d[i] -= h;
				}
				f = f + h;

				// Implicit QL transformation.

				p = d[m];
				XMType c = 1.0;
				XMType c2 = c;
				XMType c3 = c;
				XMType el1 = e[l + 1];
				XMType s = 0.0;
				XMType s2 = 0.0;
				for (int i = m - 1; i >= l; i--) {
					c3 = c2;
					c2 = c;
					s2 = s;
					g = c * e[i];
					h = c * p;
					r = hypot2(p, e[i]);
					e[i + 1] = s * r;
					s = e[i] / r;
					c = p / r;
					p = c * d[i] - s * g;
					d[i + 1] = h + s * (c * g + s * d[i]);

					// Accumulate transformation.

					for (int k = 0; k < n; k++) {
						h = V(k, i + 1);
						V(k, i + 1) = s * V(k, i) + c * h;
						V(k, i) = c * V(k, i) - s * h;
					}
				}
				p = -s * s2 * c3 * el1 * e[l] / dl1;
				e[l] = s * p;
				d[l] = c * p;

				// Check for convergence.

			} while (fabs(e[l]) > eps*tst1);
		}
		d[l] = d[l] + f;
		e[l] = 0.0;
	}

	// Sort eigenvalues and corresponding vectors.

	for (int i = 0; i < n - 1; i++) {
		int k = i;
		XMType p = d[i];
		for (int j = i + 1; j < n; j++) {
			if (d[j] < p) {
				k = j;
				p = d[j];
			}
		}
		if (k != i) {
			d[k] = d[i];
			d[i] = p;
			for (int j = 0; j < n; j++) {
				p = V(j, i);
				V(j, i) = V(j,k);
				V(j,k) = p;
			}
		}
	}
}


/** Symmetric matrix A => eigenvectors in columns of V, corresponding
eigenvalues in d. 

ONLY for SYMMETRIC MATRICES
*/

template<int n>  void eig(IMatrix<n, n>& A, IMatrix<n, n>& V, IMatrix<n, 1>& d) {
	IMatrix<n, 1> e;
	for (int i = 0; i < n; i++) { 
		for (int j = 0; j < n; j++) {
			V(i, j) = A(i, j);
		}
	}
	eigen_tred2(V, d, e);
	eigen_tql2(V, d, e);
}











#endif

