#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <valgrind/mctracer.h>

#define N 256

double H(double x)
{
	double y;
	if (x<0) y = 0;
	else if (x==0) y = 0.5;
	else y = 1;
	return y;
}

double upper(double x)
{
	double h, l, r, w;

	w = 3*sqrt(1-pow(x/7,2));
	l = (x+3)/2 - 3/7*sqrt(10)*sqrt(4-pow(x+1,2)) + 6/7*sqrt(10);
	h = (upper(abs(x+0.5)+abs(x-0.5)+6) - 11(x+3/4) + abs(x-3/4))/2;
	r = (3-x)/2 - 3/7*sqrt(10)*sqrt(4-pow(x-1,2)) + 6/7*sqrt(10);

	return (h-l)*H(x+1) + (r-h)*H(x-1) + (l-w)*H(x+3) + (w-r)*H(x-3) + w;
}

double lower(double x)
{
	return (abs(x/2) + sqrt(1-pow(abs(abs(x)-2)-1,2)) - x*x/112*(3*sqrt(33)-7) + 3*sqrt(1-pow(x/7,2)) - 3)/2 
		* ((x+4)/(abs(x+4)) - (x-4)/(abs(x-4))) - 3*sqrt(1-pow(x/7,2));
}

int main()
{
	double* m;
	m = (double*) malloc(sizeof(double)*N*N);

	// init
	for (int i=0; i<N; i++)
		for (int j=0; j<N; j++)
			m[i*N+j] = 1;

	SSIM_MATRIX_TRACING_START(m, N, N, sizeof(double), "batman - m");
	
	for (int it=50; it>0; it--) {
		//TODO	
	}

	SSIM_MATRIX_TRACING_STOP(m);
}
