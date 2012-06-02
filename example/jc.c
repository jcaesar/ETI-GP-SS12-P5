#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>

static inline
double gettime(void)
{
	struct timeval now_tv;
	gettimeofday (&now_tv, NULL);
	return ((double)now_tv.tv_sec) + ((double)now_tv.tv_usec)/1000000.0;
}

int flush_cache()
{
	/* 16MB should be enough */
	const int num_ints = 4*1024*1024;

	static int* mem = 0;
	int i, sum = 0;

	if (mem == 0)
	{
		mem = malloc(num_ints * sizeof(int));
		if (mem == 0) return 0;

		memset(mem, 1, num_ints * sizeof(int));
	}

	/* return sum of array so the accesses are not optimized away */
	for (i=0; i < num_ints; i++)
		sum += mem[i];

	return sum;
}

void init(int sz, double a[sz][sz], double value)
{
	int i, j;

	for(i=0; i<sz; i++)
		for(j=0; j<sz; j++)
			a[i][j] = value;
}

double dosum(int sz, double a[sz][sz])
{
	double sum = 0.0;
	int i, j;

	for(i=0; i<sz; i++)
		for(j=0; j<sz; j++)
			sum += a[i][j];
	return sum;
}

void init_leftright_boundary(int sz, double a[sz][sz], double value)
{
	int i, j;

	for(i=0; i<sz; i++)
	{
		a[i][0] = value;
		a[i][sz-1] = value;
	}
}

void init_diag(int sz, double a[sz][sz], double value)
{
	int i, j;

	for(i=0; i<sz; i++)
		a[i][i] = value;
}

// align at 16byte boundaries
void* mymalloc(int size, void** p)
{
	void* addr = malloc(size+16);
	if (p) *p = addr;
	return (void*)((unsigned long)addr /16*16+16);
}

/******************** Versions ***************/

/* use A for input and output, return number of iterations done */
int jc_ji(int sz, double a[sz][sz], double b[sz][sz])
{
	int i,j;

	for(j=1; j<sz-1; j++)
		for(i=1; i<sz-1; i++)
			b[i][j] = (a[i-1][j] + a[i][j-1] +
			           a[i+1][j] + a[i][j+1])/4.0;

	for(j=1; j<sz-1; j++)
		for(i=1; i<sz-1; i++)
			a[i][j] = (b[i-1][j] + b[i][j-1] +
			           b[i+1][j] + b[i][j+1])/4.0;

	return 2;
}

int jc_ij(int sz, double a[sz][sz], double b[sz][sz])
{
	int i,j;

	for(i=1; i<sz-1; i++)
		for(j=1; j<sz-1; j++)
			b[i][j] = (a[i-1][j] + a[i][j-1] +
			           a[i+1][j] + a[i][j+1])/4.0;

	for(i=1; i<sz-1; i++)
		for(j=1; j<sz-1; j++)
			a[i][j] = (b[i-1][j] + b[i][j-1] +
			           b[i+1][j] + b[i][j+1])/4.0;

	return 2;
}

__inline__
void dorow(int r, int sz, double a[sz][sz], double b[sz][sz])
{
	int j;
	for(j=1; j<sz-1; j++)
		b[r][j] = (a[r-1][j] + a[r][j-1] +
		           a[r+1][j] + a[r][j+1])/4.0;
}

int jc_w2ij(int sz, double a[sz][sz], double b[sz][sz])
{
	int r;

	dorow(1, sz, a, b);
	for(r=2; r<sz-1; r++)
	{
		dorow(r, sz, a, b);
		dorow(r-1, sz, b, a);
	}
	dorow(sz-2, sz, b, a);

	return 2;
}


/******************** Run the different versions ***************/

typedef int (*jacfunc)(int sz,
                       double a[sz][sz], double b[sz][sz]);

void run_jac(jacfunc f, const char* name, int it,
             int sz, double a[sz][sz], double b[sz][sz])
{
	double start, stop, mflops;
	int i;

	flush_cache();
	init(sz, a, 0);
	init_leftright_boundary(sz, a, 10.0);
	start = gettime();
	i=it;
	while(i>0)
		i -= (*f)(sz, a, b);
	stop = gettime();
	mflops = 4.0*(sz-2)*(sz-2)*it / (stop-start) / 1000000.0;
	fprintf(stderr, "  %s: %f sec. : %f MFlops (Sum %f)\n",
	        name, stop - start, mflops, dosum(sz, a));
	printf("%f ", mflops);
}

struct _version
{
	const char* name;
	jacfunc func;
} version[] =
{
	{ "Simple-JI", jc_ji },
	{ "Simple-IJ", jc_ij },
	{ "Weave2-IJ", jc_w2ij },
	{ 0,0 }
};


void run(int sz, int it)
{
	typedef double MAT[sz][sz];
	void *ap, *bp;
	MAT *a, *b;
	int v;

	a = (MAT*) mymalloc(sizeof(MAT), &ap);
	b = (MAT*) mymalloc(sizeof(MAT), &bp);

	init(sz, *a, 1);
	init(sz, *b, 0);

	fprintf(stderr, "Matrix side length: %d, size: %f MB (a: %p, b: %p), %d iterations\n",
	        sz, ((double)sizeof(MAT))/1000000.0, a,b, it);
	printf("%d ", sz);

	for(v=0; version[v].func !=0; v++)
		run_jac( version[v].func, version[v].name, it,
		         sz, *a, *b);

	printf("\n");

	free(ap);
	free(bp);
}

int main(int argc, char* argv[])
{
	int sz, diff;
	int sz1 = 500, sz2 = 0, steps = 0;
	int it = 300;

	if (argc>1) sz1 = atoi(argv[1]);
	if (argc>2) it = atoi(argv[2]);
	if (argc>3) sz2 = atoi(argv[3]);
	if (argc>4) steps = atoi(argv[4]);

	if ((sz2 == 0) || (sz2 < sz1)) sz2 = sz1;
	if (steps == 0)
	{
		steps = sz2-sz1+1;
		if (sz2-sz1 >= 10) steps = (sz2-sz1)/10+1;
		if (sz2-sz1 >= 100) steps = (sz2-sz1)/100+1;
	}

	if (steps <= 1) run(sz1, it);
	else
		for(sz = sz1; sz<=sz2; sz += (sz2-sz1)/(steps-1))
			run (sz, it);
}
