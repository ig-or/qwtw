/** fft library

\file complex.h
\author   Anton Klimenkov
\version 1.0

*/

#if !defined COMPLEX
#define COMPLEX


class complex{

public:
	complex(double,double);
	complex();
	complex operator+ (const complex&);
	complex operator- (const complex&);
	complex operator* (const complex&);
	void print();

private:
	double Re;
	double Im;

};

#endif