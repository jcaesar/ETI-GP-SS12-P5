#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>

#include <valgrind/mctracer.h>

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

/******************** The six alternative orderings ***************/

void mm_ijk(int sz, double a[sz][sz], double b[sz][sz], double c[sz][sz])
{
	int i,j,k;

	for(i=0; i<sz; i++)
		for(j=0; j<sz; j++)
			for(k=0; k<sz; k++)
				c[i][j] += a[i][k] * b[k][j];
}

// IJK with previous transposition

void mm_ijk_t(int sz, double a[sz][sz], double b[sz][sz], double c[sz][sz])
{
	int i,j,k;
	typedef double MAT[sz][sz];
	void *b2p;
	MAT *b2;

	b2 = (MAT*) mymalloc(sizeof(MAT), &b2p);

	for(i=0; i<sz; i++)
		for(j=0; j<sz; j++)
			(*b2)[i][j] = b[j][i];

	for(i=0; i<sz; i++)
		for(j=0; j<sz; j++)
			for(k=0; k<sz; k++)
				c[i][j] += a[i][k] * (*b2)[j][k];

	free(b2p);
}


void mm_ikj(int sz, double a[sz][sz], double b[sz][sz], double c[sz][sz])
{
	int i,j,k;

	for(i=0; i<sz; i++)
		for(k=0; k<sz; k++)
			for(j=0; j<sz; j++)
				c[i][j] += a[i][k] * b[k][j];
}

void mm_jik(int sz, double a[sz][sz], double b[sz][sz], double c[sz][sz])
{
	int i,j,k;

	for(j=0; j<sz; j++)
		for(i=0; i<sz; i++)
			for(k=0; k<sz; k++)
				c[i][j] += a[i][k] * b[k][j];
}

void mm_jki(int sz, double a[sz][sz], double b[sz][sz], double c[sz][sz])
{
	int i,j,k;

	for(j=0; j<sz; j++)
		for(k=0; k<sz; k++)
			for(i=0; i<sz; i++)
				c[i][j] += a[i][k] * b[k][j];
}

void mm_kij(int sz, double a[sz][sz], double b[sz][sz], double c[sz][sz])
{
	int i,j,k;

	for(k=0; k<sz; k++)
		for(i=0; i<sz; i++)
			for(j=0; j<sz; j++)
				c[i][j] += a[i][k] * b[k][j];
}

void mm_kji(int sz, double a[sz][sz], double b[sz][sz], double c[sz][sz])
{
	int i,j,k;

	for(k=0; k<sz; k++)
		for(j=0; j<sz; j++)
			for(i=0; i<sz; i++)
				c[i][j] += a[i][k] * b[k][j];
}

/************** A K-blocked version of IKJ ***************/

void mm_b_ikj(int sz,
              double a[sz][sz], double b[sz][sz], double c[sz][sz])
{
	int i,j,k, kk, kend, kb = sz/10+1;

	for(kk=0; kk<sz; kk+=kb)
		for(i=0; i<sz; i++)
		{
			kend = (kk+kb<sz) ? kk+kb : sz;
			for(k=kk; k<kend; k++)
				for(j=0; j<sz; j++)
					c[i][j] += a[i][k] * b[k][j];
		}
}

/************** A I-blocked version of KIJ ***************/

void mm_b_kij(int sz,
              double a[sz][sz], double b[sz][sz], double c[sz][sz])
{
	int i,j,k, ii, iend, ib = sz/10+1;

	for(ii=0; ii<sz; ii+=ib)
		for(k=0; k<sz; k++)
		{
			iend = (ii+ib<sz) ? ii+ib : sz;
			for(i=ii; i<iend; i++)
				for(j=0; j<sz; j++)
					c[i][j] += a[i][k] * b[k][j];
		}
}

/************ 2-dimensional: K/J-blocked IKJ **************/

void mm_bb_ikj(int sz,
               double a[sz][sz], double b[sz][sz], double c[sz][sz])
{
	int i,j,k, jj,kk, jend, kend, jb = sz/4+1, kb = sz/25+1;

	for(kk=0; kk<sz; kk+=kb)
	{
		kend = (kk+kb<sz) ? kk+kb : sz;
		for(jj=0; jj<sz; jj+=jb)
		{
			jend = (jj+jb<sz) ? jj+jb : sz;
			for(i=0; i<sz; i++)
				for(k=kk; k<kend; k++)
					for(j=jj; j<jend; j++)
						c[i][j] += a[i][k] * b[k][j];
		}
	}
}



/******************** Run the different versions ***************/

typedef void (*mulfunc)(int sz,
                        double a[sz][sz], double b[sz][sz], double c[sz][sz]);

void run_mul(mulfunc f, const char* name,
             int sz, double a[sz][sz], double b[sz][sz], double c[sz][sz])
{
	double start, stop, mflops;

	SSIM_FLUSH_CACHE;
	start = gettime();
	init(sz, c, 0);
	(*f)(sz, a, b, c);
	stop = gettime();
	mflops = 2.0*sz*sz*sz / (stop-start) / 1000000.0;
	fprintf(stderr, "  %s: %f sec. : %f MFlops\n", name, stop - start, mflops);
	printf("%f ", mflops);
}

struct _mmversion
{
	const char* name;
	mulfunc func;
} mmversion[] =
{
	{ "IJK", mm_ijk },
	{ "IJK-T", mm_ijk_t },
	{ "IKJ", mm_ikj },
	{ "JIK", mm_jik },
	{ "JKI", mm_jki },
	{ "KIJ", mm_kij },
	{ "KJI", mm_kji },
	{ "B-IKJ", mm_b_ikj },
	{ "B-KIJ", mm_b_kij },
	{ "BB-IKJ", mm_bb_ikj },
	{ 0,0 }
};


void run(int sz)
{
	typedef double MAT[sz][sz];
	void *ap, *bp, *cp;
	MAT *a, *b, *c;
	int v;

	a = (MAT*) mymalloc(sizeof(MAT), &ap);
	b = (MAT*) mymalloc(sizeof(MAT), &bp);
	c = (MAT*) mymalloc(sizeof(MAT), &cp);

	init(sz, *a, 1);
	init(sz, *b, 0);
	init_diag(sz, *b, 1);

	fprintf(stderr, "Matrix side length: %d, size: %f MB (a: %p, b: %p, c: %p)\n",
	        sz, ((double)sizeof(MAT))/1000000.0, a,b,c);
	printf("%d ", sz);

	for(v=0; mmversion[v].func !=0; v++)
	{
		// generate tracking identifier for each matrix
		const int len = 256;
		char name[len];
		strcpy(name, mmversion[v].name);

		strcpy(name + strlen(mmversion[v].name), " - a");
		SSIM_MATRIX_TRACING_START(a, sz, sz, sizeof(double), name);

		strcpy(name + strlen(mmversion[v].name), " - b");
		SSIM_MATRIX_TRACING_START(b, sz, sz, sizeof(double), name);

		strcpy(name + strlen(mmversion[v].name), " - c");
		SSIM_MATRIX_TRACING_START(c, sz, sz, sizeof(double), name);


		run_mul( mmversion[v].func, mmversion[v].name, sz, *a, *b, *c);

		SSIM_MATRIX_TRACING_STOP(a);
		SSIM_MATRIX_TRACING_STOP(b);
		SSIM_MATRIX_TRACING_STOP(c);
	}

	printf("\n");

	free(ap);
	free(bp);
	free(cp);
}

int main(int argc, char* argv[])
{
	int sz, diff;
	int sz1 = 500, sz2 = 0, steps = 0;

	if (argc>1) sz1 = atoi(argv[1]);
	if (argc>2) sz2 = atoi(argv[2]);
	if (argc>3) steps = atoi(argv[3]);

	if ((sz2 == 0) || (sz2 < sz1)) sz2 = sz1;
	if (steps == 0)
	{
		steps = sz2-sz1+1;
		if (sz2-sz1 >= 10) steps = (sz2-sz1)/10+1;
		if (sz2-sz1 >= 100) steps = (sz2-sz1)/100+1;
	}

	if (steps <= 1) run(sz1);
	else
		for(sz = sz1; sz<=sz2; sz += (sz2-sz1)/(steps-1))
			run (sz);
}
