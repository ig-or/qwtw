/** fft library

\file complex.cpp
\author   Anton Klimenkov
\version 1.0

*/


#include "complex.h"
#include "stdio.h"

complex::complex(double a,double b){
	Re=a;
	Im=b;
}

complex::complex(){
	Re=0.0;
	Im=0.0;
}

complex complex::operator+(const complex& z){
	complex c;
	c.Re=this->Re+z.Re;
	c.Im=this->Im+z.Im;
	return c;
}

complex complex::operator -(const complex &z){
	complex c;
	c.Re=this->Re-z.Re;
	c.Im=this->Im-z.Im;
	return c;
}

complex complex::operator *(const complex &z){
	complex c;
	c.Re=this->Re*z.Re+this->Im*z.Im;
	c.Im=-(this->Im*z.Re+this->Re*z.Im);
	return c;
}

void complex::print(){
	printf("%f+i*%f\n",Re,Im);
}