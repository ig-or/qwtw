
/**

	3D line data handling.
	

	\file qfpath.h
	\author   Igor Sandler
	\date    Jul 2009
	\version 1.0
	
*/



#ifndef QFPATH_H_FILE
#define QFPATH_H_FILE

#include <QVector>
#include <math.h>

#ifdef qwtflib_EXPORTS	
#define qwtflib_API Q_DECL_EXPORT //__declspec(dllexport)
#else
#define qwtflib_API Q_DECL_IMPORT // __declspec(dllimport)
#endif


struct	QFTriple {
	double x[3];
	inline double& operator[](const int i) { return x[i]; }
	inline double operator()(const int i) const { return x[i]; }
	QFTriple& operator=(const QFTriple& q) {
		if (this == &q) return *this;
		memcpy(x, q.x, 3*8);
		return *this;
	}
	QFTriple	operator+(const QFTriple& v) const {
		QFTriple q;
		q.x[0] = x[0] + v.x[0];  q.x[1] = x[1] + v.x[1]; q.x[2] = x[2] + v.x[2]; 
		return q;
	}
	QFTriple	operator-(const QFTriple& v) const {
		QFTriple q;
		q.x[0] = x[0] - v.x[0];  q.x[1] = x[1] - v.x[1]; q.x[2] = x[2] - v.x[2]; 
		return q;
	}
	double length()  {return sqrt(x[0]*x[0] + x[1]*x[1] + x[2]*x[2]); }
        //friend static QFTriple qfcross(const QFTriple& v1, const QFTriple& v2);
};
QFTriple qfcross(const QFTriple& v1, const QFTriple& v2);

struct QFData {
	QFTriple	p; ///< point
	int			i; ///< index
	double		r; ///< radius
	double		c; ///<   color
};


class qwtflib_API QFPath {
public:

	QVector<QFData>	data1;
	QVector<QFData>	data2;
	double	bounds[2][3];
	double	length;
	double res;
	int counter1, counter2, N;
	double maxColor, minColor, deltaColor;

	QFPath(); // {}
	/** put points into the "data1".   distance between  points > res.
	@param x all the points. x[0..N-1] === X, x[N..2N-1] === Y, x[2N..3N-1] === Z
	@param[in] N number of points
	@param[in] res
	@return number of points in data1.

	*/
	int work1(double* x, int N_, double res_, double* r = 0, double* c = 0);
	int work2();
	/**   return true if distance from point x to line x1-x2 is more than "res"

	*/
	//bool dxTest(const QFTriple& x1, const QFTriple& x2, const QFTriple& x);


};


#endif

