/*
 * Event consumer for mctracer: simple cache simulator.
 * For ETI @ TUM, (C) 2011 Josef Weidendorfer
 */
/* ----------------------------------------------------------------*/

#include "simplesim.h"

#define _GNU_SOURCE
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include "pub_tool_libcbase.h" // VG_(strlen)
#include "pub_tool_libcfile.h" // VG_(open), VG_(write)
#include "pub_tool_vki.h" // VKI_O_TRUNC ... 
#include "pub_tool_mallocfree.h" // VG_(malloc)
#include "pub_tool_libcprint.h" // VG_(printf)
#include "pub_tool_libcassert.h" // VG_(tool_panic)

/*
 * Simulator for a shared cache
 */

// cache line size
#define LINESIZE     64

typedef struct _cacheline
{
	Addr tag;            // 64 bit architecture
} Cacheline;

Cacheline *cache;
bool init_done = false;
Int cachelines, setsize, sets;

void ssim_init(Int sets_, Int setsize_)
{
	if(init_done) return;
	init_done = true;

	sets = sets_;
	setsize = setsize_;
	cachelines = sets*setsize;

	cache = VG_(malloc)("cache", sizeof(Cacheline) * cachelines);

	if(clo_ssim_max_pattern_length > MAX_MAX_PATTERN_LENGTH)
	{
		VG_(printf)("mctracer: --max-pattern-length=%d too large. Cropping to %d\n", clo_ssim_max_pattern_length, MAX_MAX_PATTERN_LENGTH);
		clo_ssim_max_pattern_length = MAX_MAX_PATTERN_LENGTH;
	}
	else if(clo_ssim_max_pattern_length < 4)
	{
		VG_(printf)("mctracer: --max-pattern-length=%d too small. Extending to 4\n", clo_ssim_max_pattern_length);
		clo_ssim_max_pattern_length = 4;
	}

	if(clo_ssim_max_patterns_per_matrix > 255)
	{
		VG_(printf)("mctracer: --max-patterns-per-matrix=%d too large. Cropping to 256\n", clo_ssim_max_patterns_per_matrix);
		clo_ssim_max_patterns_per_matrix = 255;
	}
	else if(clo_ssim_max_pattern_length < 4)
	{
		VG_(printf)("mctracer: --max-patterns-per-matrix=%d too small. Extending to 4\n", clo_ssim_max_patterns_per_matrix);
		clo_ssim_max_pattern_length = 4;
	}



	ssim_flush_cache();
}

void ssim_flush_cache(void)
{
	int i;

	for(i=0; i<cachelines; i++)
	{
		cache[i].tag = 0;
	}

}

// a reference into a set of the cache, return 1 on hit
static int cache_setref(int set_no, Addr tag)
{
	int i, j;
	Cacheline* set = cache + set_no * setsize;

	/* Test all lines in the set for a tag match
	 * If the tag is another than the MRU, move it into the MRU spot
	 * and shuffle the rest down.
	 */
	for (i = 0; i < setsize; i++)
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
	for (j = setsize - 1; j > 0; j--)
	{
		set[j].tag = set[j - 1].tag;
	}
	set[0].tag = tag;

	return 0;
}

// a reference at address <a> with size <s>, return 1 on hit
int cache_ref(Addr a, int size)
{
	int  set1 = ( a         / LINESIZE) & (sets-1);
	int  set2 = ((a+size-1) / LINESIZE) & (sets-1);
	Addr tag  = a / LINESIZE / sets;
	Addr tag2;
	int res1, res2;

	/* Access entirely within line. */
	if (set1 == set2)
		return cache_setref(set1, tag);

	/* Access straddles two lines. */
	/* NOTE: We assume an access not overlapping >2 cache lines ! */
	tag2  = (a+size-1) / LINESIZE / sets;

	/* the call updates cache structures as side effect */
	res1 =  cache_setref(set1, tag);
	res2 =  cache_setref(set2, tag2);
	/* return 0 (=Miss) if at least one result was 0 */
	return res1 * res2;
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

	if(name != NULL && VG_(strlen)(name) > 0)
	{
		matr->name = (char*) VG_(malloc)("name", VG_(strlen)(name)*sizeof(char));
		VG_(strcpy)(matr->name, name);
	}
	else
	{
		/** use VG_(malloc) so we don't an extra condition for the VG_(free) in tr_write.c **/
		matr->name = (char*) VG_(malloc)("empty name", sizeof(char));
		char* emptyName = "\n";
		VG_(strcpy)(matr->name, emptyName);
	}

	// size of each element of the matrix in bytes
	matr->ele_size = ele_size;
	matr->m = m;
	matr->n = n;

	/* initialize memory access infos */
	matr->load_access_data.last_access.n = SHRT_MIN;
	matr->load_access_data.last_access.m = SHRT_MIN;
	matr->store_access_data.last_access.n = SHRT_MIN;
	matr->store_access_data.last_access.m = SHRT_MIN;
	matr->last_access.n = SHRT_MIN;
	matr->last_access.m = SHRT_MIN;
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

	matr->loads.hits = 0;
	matr->stores.hits = 0;
	matr->loads.misses = 0;
	matr->stores.misses = 0;

	// pattern finding stores
	matr->access_buffer = (access_event*) VG_(malloc)("matrix access event buffer", MATRIX_ACCESS_ANALYSIS_BUFFER_LENGTH*sizeof(access_event));
	matr->access_event_count = 0;
	matr->access_patterns = VG_(malloc)("matrix patterns", clo_ssim_max_patterns_per_matrix*sizeof(access_pattern));
	VG_(memset)(matr->access_patterns, 0, clo_ssim_max_patterns_per_matrix*sizeof(access_pattern));
	matr->current_pattern = 0;
	matr->current_sequence_length = 0;

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
		VG_(printf)("mctracer: call to SSIM_MATRIX_TRACING_STOP with invalid address %lu\n", addr);
		return false;
	}

	// add remaining events to sequences
	process_pattern_buffer(matr);
	// free now unused memory
	VG_(free)(matr->access_buffer);
	matr->access_buffer = NULL;

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
	for (i=0; i < last_traced_mtr_offset; ++i)
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
