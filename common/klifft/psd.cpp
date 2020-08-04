/** fft library

\file psd.cpp
\author   Anton Klimenkov
\version 1.0

*/

#include "psd.h"
#include "math.h"
#include "stdio.h"

#define SQR(a) (a)*(a)
#define pi 3.141592653589793

Psd::Psd(){
    psd = NULL;
    frequencies = NULL;
}

Psd::~Psd(){
    if (psd != NULL)
        delete[] psd;
    if (frequencies != NULL)
        delete[] frequencies;
}

//k - number of intervals
//M - number of elements per interval
void Psd::compute(double *data, int length,double dt,  int k){
	int i,j,a;
    int dataPos = 0; //where in data we are!

    int N;//the number of points we are going to use (is power of two)
    int M;//number of points in one averaging interval

    //number of points must be equal to power of 2
    int powerTwo = 0;
    int tmp = length;
    while ((tmp = tmp / 2) >= 1)
        powerTwo++;  
    N = (int)pow(2.0,powerTwo);
    //
    
	M = N/k;
	int m=M/2;
	size = m;
    double* d = new double[M];
	double* d1 = new double[m];
	double* d2 = new double[m];

	double w;//variable to store result of window function;
	double sumw = 0.0;//"window squared and summed". See Numerical Recipes in C p.533
	double norm = 0.0;//normalisation value (we calculate mean squared Psd)

    //calculating mean
    double mean = 0.0;
    for (i=0;i<length;i++)
        mean += data[i];
    mean /= length;
    //

    //arrays of frequencies
    if (frequencies != NULL)
        delete[] frequencies;

    frequencies = new double[m];
	for (i=0;i<m;i++) 
		frequencies[i] = i/(double)M / dt;

    //psd itself
    //needs to be checked if psd is already created
    if (psd != NULL)
        delete[] psd;

    psd = new double[m];
	for (i=0;i<m;i++)
        psd[i]=0.0;

	for (i=0;i<m;i++)// "window summed and squared" further used to normalise Psd
		sumw += SQR(window(i,m));//counted for half of interval.
	sumw*=2;
	for (i=0;i<m;i++){
        //fscanf(f,"%lf ",&d2[j][i]); 
        d2[i] = data[i];
        d2[i] -= mean;
	}
    dataPos = m;
	for(i=0;i<2*k-1;i++){//reading data from file
		for (j=0;j<m;j++){
			d1[j]=d2[j];
            d2[j] = data[dataPos + j];
            d2[j] -= mean;
		}
        dataPos += m;
        for (j=0;j<m;j++){//applying Bartlett Window function (if not manually changed) 
            w = window(j,m);
            d[j] = d1[j]*w;
            d[M-1-j] = d2[m-1-j]*w;
        }
        real_four(d-1,M,1);//see class FFT
        psd[0] += d[0]*d[0];
        psd[m-1] += d[1]*d[1];
        for (j=1;j<m-1;j++){
            a = 2*j;
            psd[j] += 2*(SQR(d[a])+SQR(d[a+1]));//
        }
	}
	norm = sumw*N*(2*k-1);//normalizing.
	for (i=0;i<m;i++){
        //psd[i]=sqrt(psd[i]) /sqrt(norm);
        psd[i]=psd[i] * dt;
	}
	delete[] d;
	delete[] d1;
	delete[] d2;
}

//Function for windowing data
double Psd::window(int j,int m){
	double res;
	double f=(double)m;//to avoid uncertainties with stupid fabs
	res=1.0-fabs(((j+1)-f)/f);//Bartlett (triangle)
	//res=1.0;//Square
	//res=1.0-SQR(((j+1)-f)/f);//Welch
	return res;
}
