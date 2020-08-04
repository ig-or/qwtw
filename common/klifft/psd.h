/** fft library

\file psd.h
\author   Anton Klimenkov
\version 1.0

*/

#if !defined P_S_D
#define P_S_D

#include "fft.h"
#include "stdio.h"

class Psd : public Fft{
public:
    Psd();
	~Psd();

    // ==== functions ====
    // [double *data, int length of data,double period of sampling, int number of averaging intervals]
	void compute(double*,int,double,int);//see Numerical Recipes in C for clarity. It was my inspiration.
	double window(int,int);//function for windowing data

    // ==== variables ====
	double *psd;//array for PSD of each of n devices
	double *frequencies;//array of frequencies in Hz. for ease of plotting result
	int size;
};

#endif
