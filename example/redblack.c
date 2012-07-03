#include <stdio.h>
#include <stdlib.h>

#include <valgrind/mctracer.h>

int main(int argc, char* argv[])
{
	double* m;
	int i, j, it;
	double sum = 0.0;
	int size = 1024;
	if(argc == 2) size = atoi(argv[1]);

	m = (double*) malloc(sizeof(double)*size*size);
	SSIM_MATRIX_TRACING_START(m, size, size, sizeof(double), "redblack - m");

	// init points
	for(i=0; i<size; i++)
		for(j=0; j<size; j++)
			m[i*size+j] = 0.0;
	// set left/right border
	for(i=0; i<size; i++)
	{
		m[ i*size + 0        ] = 10.0;
		m[ i*size + (size-1) ] = 10.0;
	}

	// run 50 iterations
	for(it=0; it<50; it++)
	{

		// update inner black points
		for(i=1; i<size-1; i++)
			for(j=1+(i%2); j<size-1; j+=2)
				m[i*size+j] = ( m[(i-1)*size +j] +
				                m[(i+1)*size +j] +
				                m[ i*size + j-1] +
				                m[ i*size + j+1] )/4.0;

		// update inner red points
		for(i=1; i<size-1; i++)
			for(j=1+((i+1)%2); j<size-1; j+=2)
				m[i*size+j] = ( m[(i-1)*size +j] +
				                m[(i+1)*size +j] +
				                m[ i*size + j-1] +
				                m[ i*size + j+1] )/4.0;
	}

	// print checksum
	for(i=0; i<size; i++)
		for(j=0; j<size; j++)
			sum += m[i*size+j];

	printf("Sum: %f\n", sum);

	SSIM_MATRIX_TRACING_STOP(m);

	return 1;
}
