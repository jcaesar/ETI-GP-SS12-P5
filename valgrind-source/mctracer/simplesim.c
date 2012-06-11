/*
 * Event consumer for mctracer: simple cache simulator.
 * For ETI @ TUM, (C) 2011 Josef Weidendorfer
 */

#include "simplesim.h"

#define _GNU_SOURCE
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include "filemanip.h"
#include "pub_tool_mallocfree.h" // VG_(malloc)
#include "pub_tool_libcprint.h" // VG_(printf)
#include "pub_tool_libcassert.h" // VG_(tool_panic)

/* ----------------------------------------------------------------*/

/**
 * utitlity functions, because of the missing libc
 */

static int ssim_strlen(char* str)
{
	int len;

	for (len = 0; *(str+len) != '\0'; len++);

	return len;
}

static void swap(matrix_access_method* a, matrix_access_method* b)
{
	matrix_access_method t = *a;
	*a = *b;
	*b = t;
}

static void ssim_qsort(matrix_access_method arr[], int begin, int end)
{
	if (end > begin + 1)
	{
		unsigned int piv = arr[begin].hits + arr[begin].misses;
		int l = begin + 1, r = end;

		while (l < r)
		{
			unsigned int ele = arr[l].hits + arr[l].misses;

			if (ele > piv)
			{
				l++;
			}
			else
			{
				swap(&arr[l], &arr[--r]);
			}
		}

		swap(&arr[--l], &arr[begin]);
		ssim_qsort(arr, begin, l);
		ssim_qsort(arr, r, end);
	}
}

#define MATRIX_LOAD 'L'
#define MATRIX_STORE 'S'

#define byte unsigned char
#define ushort unsigned short

/* ----------------------------------------------------------------*/

/*
 * Simulator for a shared cache
 */

// Cache with 8192 cache lines a 64 byte = 1 MB cache size
// Associativity 16 (= number of cache lines per set)
#define LINESIZE     64
#define CACHELINES 8192
#define SETSIZE      16

// derived parameter
#define SETS (CACHELINES / SETSIZE)

typedef struct _cacheline
{
	Addr tag;            // 64 bit architecture
} Cacheline;

Cacheline cache[CACHELINES];

bool init_done = false;

void ssim_init(void)
{
	if(init_done) return;
	init_done = true;

	ssim_flush_cache();
}

void ssim_flush_cache(void)
{
	int i;

	for(i=0; i<CACHELINES; i++)
	{
		cache[i].tag = 0;
	}

}

// a reference into a set of the cache, return 1 on hit
static int cache_setref(int set_no, Addr tag)
{
	int i, j;
	Cacheline* set = cache + set_no * SETSIZE;

	/* Test all lines in the set for a tag match
	 * If the tag is another than the MRU, move it into the MRU spot
	 * and shuffle the rest down.
	 */
	for (i = 0; i < SETSIZE; i++)
	{
		if (tag == set[i].tag)
		{
			for (j = i; j > 0; j--)
			{
				set[j].tag = set[j - 1].tag;
			}
			set[0].tag = tag;

			return 1;
		}
	}

	/* A miss;  install this tag as MRU, shuffle rest down. */
	for (j = SETSIZE - 1; j > 0; j--)
	{
		set[j].tag = set[j - 1].tag;
	}
	set[0].tag = tag;

	return 0;
}

// a reference at address <a> with size <s>, return 1 on hit
static int cache_ref(Addr a, int size)
{
	int  set1 = ( a         / LINESIZE) & (SETS-1);
	int  set2 = ((a+size-1) / LINESIZE) & (SETS-1);
	Addr tag  = a / LINESIZE / SETS;
	Addr tag2;
	int res1, res2;

	/* Access entirely within line. */
	if (set1 == set2)
		return cache_setref(set1, tag);

	/* Access straddles two lines. */
	/* NOTE: We assume an access not overlapping >2 cache lines ! */
	tag2  = (a+size-1) / LINESIZE / SETS;

	/* the call updates cache structures as side effect */
	res1 =  cache_setref(set1, tag);
	res2 =  cache_setref(set2, tag2);
	/* return 0 (=Miss) if at least one result was 0 */
	return res1 * res2;
}

/* ----------------------------------------------------------------*/

traced_matrix traced_matrices [MAX_MATRIX_COUNT];
traced_matrix* traced_matrices_index[MAX_MATRIX_COUNT];

// number of traced matrices
int traced_matrices_count = 0;
// number of stopped matrice traces
int stopped_matrices_count = 0;

static traced_matrix* find_matrix(Addr access)
{
	int i;
	int tmax = traced_matrices_count - stopped_matrices_count;

	for(i=0; i < tmax; ++i)
	{
		if (traced_matrices_index[i]->start <= access &&
		        traced_matrices_index[i]->end > access)
		{
			return traced_matrices_index[i];
		}
	}

	return 0;
}

static void update_matrix_access_stats(traced_matrix* matr, int is_hit, Addr addr, SizeT size)
{
	if(matr)
	{
		// transform addr to (m,n) representation
		Addr tmp = (addr - (*matr).start) / (*matr).ele_size;
		unsigned short n = tmp % (*matr).m;
		unsigned short m = (tmp - n) / (*matr).m;

		// ignore the first access
		if ((*matr).access_data.last_access.m != SHRT_MIN && (*matr).access_data.last_access.n != SHRT_MIN)
		{

			// calculate the current access method
			short offset_n = n - (*matr).access_data.last_access.n;
			short offset_m = m - (*matr).access_data.last_access.m;

			int i;
			int amc = (*matr).access_data.access_methods_count;
			bool found = false;

			// look for the access method and increase the hit/miss count
			for (i=0; i < amc; ++i)
			{
				matrix_access_method* am = (*matr).access_data.access_methods+i;

				if (am->offset_m == offset_m && am->offset_n == offset_n)
				{
					// we got a matching access method
					// ... now check for hit or miss
					if (is_hit)
					{
						am->hits++;
					}
					else
					{
						am->misses++;
					}

					found = true;
					break;
				}
			}

			// if no access method was found => add a new one
			if (!found)
			{
				matrix_access_method* am = ((*matr).access_data.access_methods + (*matr).access_data.access_methods_count);
				am->offset_m = offset_m;
				am->offset_n = offset_n;

				if (is_hit)
				{
					// it's a hit
					am->hits = 1;
					am->misses = 0;
				}
				else
				{
					// it's a miss
					am->misses = 1;
					am->hits = 0;
				}

				(*matr).access_data.access_methods_count++;

				if ((*matr).access_data.access_methods_count >= MAX_MATRIX_ACCESS_METHODS)
				{
					VG_(tool_panic)("Max. number of access methods exceeded.");
				}
			}
		}

		// update
		(*matr).access_data.last_access.n = n;
		(*matr).access_data.last_access.m = m;

	}
}

static void update_matrix_stats(Addr addr, SizeT size, char type)
{
	traced_matrix* matr = find_matrix(addr);

	if (matr)
	{
		int is_hit = cache_ref(addr, size);
		// update access method stats
		update_matrix_access_stats(matr, is_hit, addr, size);

		// update general access stats
		int offset = (addr - matr->start) / matr->ele_size;
		if (is_hit)
		{
			if (type == MATRIX_LOAD)
			{
				matr->load_count[offset].hits++;
			}
			else
			{
				matr->store_count[offset].hits++;
			}
		}
		else
		{
			if (type == MATRIX_LOAD)
			{
				matr->load_count[offset].misses++;
			}
			else
			{
				matr->store_count[offset].misses++;
			}
		}
	}
}

VG_REGPARM(2) void ssim_load(Addr addr, SizeT size)
{
	update_matrix_stats(addr, size, MATRIX_LOAD);
}

VG_REGPARM(2) void ssim_store(Addr addr, SizeT size)
{
	update_matrix_stats(addr, size, MATRIX_STORE);
}

bool ssim_matrix_tracing_start(Addr addr, unsigned short m, unsigned short n, unsigned short ele_size, char* name)
{
	if(traced_matrices_count >= MAX_MATRIX_COUNT)
	{
		char msg[100];
		VG_(sprintf)(msg, "Max. number of matrices exceeded. Can't track more than %d matrices", MAX_MATRIX_COUNT);
		VG_(tool_panic)(msg);

		return false;
	}

	traced_matrix* matr = traced_matrices + traced_matrices_count;

	/* store general information about the matrix */
	matr->start = addr;
	matr->end = addr + m*n*ele_size;
	matr->name = name;
	// size of each element of the matrix in bytes
	matr->ele_size = ele_size;
	matr->m = m;
	matr->n = n;

	/* initialize memory access infos */
	matr->access_data.last_access.n = SHRT_MIN;
	matr->access_data.last_access.m = SHRT_MIN;
	matr->load_count = (element_access_count*) VG_(malloc)("matrix load count", m*n*sizeof(element_access_count));
	matr->store_count = (element_access_count*) VG_(malloc)("matrix store count", m*n*sizeof(element_access_count));

	// initialize every element with 0
	int i, j, offset;
	for(i = 0; i < m; ++i)
	{
		for(j = 0; j < n; ++j)
		{
			offset = i*n + j;

			matr->load_count[offset].hits = 0;
			matr->load_count[offset].misses = 0;

			matr->store_count[offset].hits = 0;
			matr->store_count[offset].misses = 0;
		}
	}

	/* Update the matrix index */

	if (traced_matrices_count == 1 || stopped_matrices_count == 0)
	{
		traced_matrices_index[traced_matrices_count] = matr;
	}
	else
	{
		/**
		 * the idea is to store the new element
		 * at the position of the first "untraced"
		 * element and move the "untraced" element
		 * to the end of the array
		 */

		// put the first unused matrix to the end of the array
		traced_matrices_index[traced_matrices_count] = traced_matrices_index[traced_matrices_count - stopped_matrices_count];

		// store the ptr to the newly traced matrix at the just freed position
		traced_matrices_index[traced_matrices_count - stopped_matrices_count] = matr;
	}

	traced_matrices_count++;

	return true;
}

bool ssim_matrix_tracing_stop(Addr addr)
{
	// the matrix to stop tracing
	traced_matrix* matr = find_matrix(addr);

	if(!matr)
	{
		return false;
	}

	/* We'll store "untraced" matrices at
	 * the end of the array, so that we'll find the
	 _* actively traced ones faster.
	 */

	// the last actively traced matrix in the array
	int last_traced_mtr_offset = traced_matrices_count - stopped_matrices_count - 1;
	traced_matrix* tmp_tm = traced_matrices_index[last_traced_mtr_offset];

	// look for the mtr from 0 to last_traced_mtr_offset-1, so
	// that we don't swap a matrix with itself
	int i;
	for (i=0; i < last_traced_mtr_offset-1; ++i)
	{
		if (traced_matrices_index[i]->start == matr->start)
		{
			traced_matrices_index[last_traced_mtr_offset] = matr;
			traced_matrices_index[i] = tmp_tm;
		}
	}

	stopped_matrices_count++;
	return true;
}

void ssim_save_stats(char* fname)
{
	uint64_t fd = hxopen(fname, HX_WRONLY|HX_CREAT|HX_TRUNC, 0644);

	/**
	 * FILE HEADER (4 bytes)
	 */

	// our magix number OxAFFE
	hxputc(fd, 0xAF);
	hxputc(fd, 0xFE);

	// version number
	hxputc(fd, 0x01);
	// total number of matrices
	hxputc(fd, (byte)traced_matrices_count);

	/**
	 * MATRICES HEADER (N*17 bytes)
	 */

	// MH:MIBADR => initalize with the DH and MH size
	unsigned int mibaddr = 4 + traced_matrices_count*17;
	int i;
	for (i = 0; i < traced_matrices_count; ++i)
	{
		// MH:GM (2 bytes)
		hxwrite(fd, &(traced_matrices[i].m), sizeof(ushort));

		// MH:GN (2 bytes)
		hxwrite(fd, &(traced_matrices[i].n), sizeof(ushort));

		// MH:AZ (1 byte), limited to 8
		int tmp = traced_matrices[i].access_data.access_methods_count;
		byte accm_count = tmp < 8 ? (byte)tmp : 8;
		hxputc(fd, accm_count);

		// MH:ADR (8 byte)
		hxwrite(fd, &(traced_matrices[i].start), sizeof(Addr));

		// MH:MIBADR (4 byte)
		hxwrite(fd, &mibaddr, sizeof(unsigned int));

		// calculate the address of the next MIB
		// matrices
		mibaddr += 2 * traced_matrices[i].m * traced_matrices[i].n;
		// access methods
		mibaddr += 12 * accm_count;
		// name + \0
		mibaddr += ssim_strlen(traced_matrices[i].name) + 1;
	}

	/**
	 * MIB - Matrix Informationblock
	 */
	for (i = 0; i < traced_matrices_count; ++i)
	{
		int rows = traced_matrices[i].m;
		int cols = traced_matrices[i].n;

		byte* loads_array = (byte*) VG_(malloc)("loads byte array", rows * cols);
		byte* stores_array = (byte*) VG_(malloc)("stores byte array", rows * cols);

		/**
		 * Bytearray (BA)
		 */
		int l;
		for(l = 0;  l < rows; ++l)
		{
			int k;
			for (k = 0; k < cols; ++k)
			{
				int offset = l*rows + k;

				// TODO clean up duplicates

				/**
				 * LOADS
				 */
				element_access_count load_count = traced_matrices[i].load_count[offset];

				if (load_count.misses == 0 && load_count.hits == 0)
				{
					// no access at all
					loads_array[offset] = BA_NO_ACCESS_VAL;
				}
				else
				{
					loads_array[offset] = (byte) (BA_MAX_HIT_VAL * ((float)load_count.hits) / (load_count.hits + load_count.misses));
				}

				/**
				 * STORES
				 */
				element_access_count store_count = traced_matrices[i].store_count[offset];

				if (store_count.misses == 0 && store_count.hits == 0)
				{
					// no access at all
					stores_array[offset] = BA_NO_ACCESS_VAL;
				}
				else
				{
					stores_array[offset] = (byte) (BA_MAX_HIT_VAL * ((float)store_count.hits) / (store_count.hits + store_count.misses));
				}
			}
		}

		hxwrite(fd, loads_array, rows*cols);
		hxwrite(fd, stores_array, rows*cols);

		// free all that memory
		VG_(free)(loads_array);
		VG_(free)(stores_array);
		VG_(free)(traced_matrices[i].load_count);
		VG_(free)(traced_matrices[i].store_count);

		/**
		 * Access methods (ZA) (N*12 bytes)
		 */

		int accm_count = traced_matrices[i].access_data.access_methods_count;

		if (accm_count > 8)
		{
			// sort the access methods according to their cumulative hits and misses
			ssim_qsort(traced_matrices[i].access_data.access_methods, 0, traced_matrices[i].access_data.access_methods_count);
		}

		accm_count = accm_count > 8 ? 8 : accm_count;

		int k;
		for (k = 0; k < accm_count; ++k)
		{
			matrix_access_method accm = traced_matrices[i].access_data.access_methods[k];

			// ZA:OM
			hxwrite(fd, &(accm.offset_m), sizeof(ushort));

			// ZA:ON
			hxwrite(fd, &(accm.offset_n), sizeof(ushort));

			// ZA:AH
			hxwrite(fd, &(accm.hits), sizeof(unsigned int));

			// ZA:AM
			hxwrite(fd, &(accm.misses), sizeof(unsigned int));
		}

		/**
		 * Name (NA)
		 */

		// matrix name + \0
		hxwrite(fd, traced_matrices[i].name, ssim_strlen(traced_matrices[i].name)+1);
	}

	hxclose(fd);
}
