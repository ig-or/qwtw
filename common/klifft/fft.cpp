/** fft library

\file fft.cpp
\author   Anton Klimenkov
\version 1.0

*/

#include "fft.h"
#include "complex.h"
#include "math.h"
#include "stdio.h"

//Used for fast calculation of Fft of complex array.
//For clarity see Numerical recipes in C
void Fft::four(double *data,int nn,int isign){
	int n,mmax=2,m,j,istep,i;
	double wtemp,wr,wpr,wpi,wi,theta; 
	double tempr,tempi;
	double C;
	n = nn*2;
	j=1;
	for (i=1;i<n;i+=2){ 
		if (j > i){ 
			C = data[j];
			data[j] = data[i];
			data[i] = C;
			C = data[j+1];
			data[j+1] = data[i+1];
			data[i+1] = C;
		}
		m=nn;
		while (m >= 2 && j > m) {
			j -= m;
			m /= 2;
		}
		j += m;
	}
	while (n > mmax) {
		istep=mmax * 2;
		theta=isign*(6.28318530717959/mmax); 
		wtemp=sin(0.5*theta);
		wpr = -2.0*wtemp*wtemp;
		wpi=sin(theta);
		wr=1.0;
		wi=0.0;
		for (m=1;m<mmax;m+=2) { 
			for (i=m;i<=n;i+=istep) {
				j=i+mmax; 
				tempr=wr*data[j]-wi*data[j+1]; 
				tempi=wr*data[j+1]+wi*data[j];
				data[j]=data[i]-tempr;
				data[j+1]=data[i+1]-tempi;
				data[i] += tempr;
				data[i+1] += tempi;
			}
		wr=(wtemp=wr)*wpr-wi*wpi+wr; 
		wi=wi*wpr+wtemp*wpi+wi;
		}
	mmax=istep;
	}
}



//replaces input array "data" with FT positive frequencies half.
//The real-valued first and last components of the complex transform are returned as 1st & 2nd elements
//the other complex components are stored in inversed sequence for ease of calculating PSD 
//(or because of a mistake :c)).
//N - number of elements in array
void Fft::real_four(double* data,int N,int isign){
	int i,i1,i2,i3,i4,np3;
	double c1=0.5,c2,h1r,h1i,h2r,h2i;
	double wr,wi,wpr,wpi,wtemp,theta; 
	theta=3.141592653589793/(double) (N>>1); 
	if (isign ==1){
		c2 = -0.5;
		four(data,N/2,1); 
	}
	else{
		c2 = 0.5;
		theta = -theta;
	}
	wtemp=sin(0.5*theta);
	wpr = -2.0*wtemp*wtemp;
	wpi=sin(theta);
	wr=1.0+wpr;
	wi=wpi;
	np3=N+3;
	for (i=2;i<=(N>>2);i++) { 
		i4=1+(i3=np3-(i2=1+(i1=i+i-1)));
		h1r=c1*(data[i1]+data[i3]); 
		h1i=c1*(data[i2]-data[i4]);
		h2r = -c2*(data[i2]+data[i4]);
		h2i=c2*(data[i1]-data[i3]);
		data[i1]=h1r+wr*h2r-wi*h2i; 
		data[i2]=h1i+wr*h2i+wi*h2r;
		data[i3]=h1r-wr*h2r+wi*h2i;
		data[i4]=-h1i+wr*h2i+wi*h2r;
		wr=(wtemp=wr)*wpr-wi*wpi+wr; 
		wi=wi*wpr+wtemp*wpi+wi;
	}
	if (isign == 1){
		data[1] = (h1r=data[1])+data[2];
		data[2] = h1r-data[2];
	} else {
		data[1]=c1*((h1r=data[1])+data[2]);
		data[2]=c1*(h1r-data[2]);
		four(data,N/2,-1);
		for (i=0;i<N;i++)
			data[i]*=(2/N);
	}
}

