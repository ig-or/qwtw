/** fft library

\file fft.h
\author   Anton Klimenkov
\version 1.0

*/

#if !defined FastF
#define FastF

class complex;

class Fft{

public:
	void inverse();//see Numerical recipes in C
	void four(double*,int,int);//for complex data
	void real_four(double*,int,int);//for NOT complex data

private:
};

#endif
