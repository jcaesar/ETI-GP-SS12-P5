/*
 * Event consumer for mctracer: simple cache simulator.
 * For ETI @ TUM, (C) 2011 Josef Weidendorfer
 */

#include "simplesim.h"

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h> // memcpy
#include "pub_tool_libcprint.h" // printf
#include "pub_tool_libcassert.h" // tool_panic

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

/* global counters for cache simulation */
int loads = 0, stores = 0, lmisses = 0, smisses = 0;

typedef struct _traced_matrix {
    Addr start; // TODO: We will want to save size and a pointer to the infostring in this, too.
	Addr end;
    char* name;
} traced_matrix;

traced_matrix matrices [MAX_MATRIX_COUNT]; 

// number of matrices in the array "matrices"
int matrix_count = 0;

// if you want, you can implement something binary-searchish for this. The table is probably going to have 3 elements, though...
traced_matrix * find_matrix(Addr access); // -Wmissing-prototypes is idiotic
traced_matrix * find_matrix(Addr access)
{
	traced_matrix * it  = matrices,
	              * end = matrices + matrix_count;
	for(; it < end; ++it) 
		if(it->start <= access && access < it->end)
		   return it;
	return 0;
}

VG_REGPARM(2) void ssim_load(Addr addr, SizeT size)
{
	if(!find_matrix(addr)) return;
	int res;
	res = cache_ref(addr, size);
//	VG_(printf)(" > Load at %p, size %2d: %s\n", (void*) addr, size, res ? "Hit ":"Miss");
	loads++;
	if (res == 0) lmisses++;
}

VG_REGPARM(2) void ssim_store(Addr addr, SizeT size)
{
	if(!find_matrix(addr)) return;
	int res;
	res = cache_ref(addr, size);
//	VG_(printf)(" > Store at %p, size %2d: %s\n", (void*) addr, size, res ? "Hit ":"Miss");
	stores++;
	if (res == 0) smisses++;
}

/**
 * Keeps track of a new matrix.
 */
bool ssim_matrix_allocated(Addr addr, int x, int y, int elsize, char* name)
{
	if(matrix_count >= MAX_MATRIX_COUNT) {
		char msg[100];
        VG_(sprintf)(msg, "Max. number of matrices exceeded. Can't track more than %d matrices", MAX_MATRIX_COUNT);
        VG_(tool_panic)(msg);

        return false;
    }

    (matrices + matrix_count)->start = addr;
	(matrices + matrix_count)->end = addr + x*y*elsize;
	(matrices + matrix_count)->name = name;

    ++matrix_count;
	
    return true;
}

bool ssim_matrix_freed(Addr addr)
{
	traced_matrix* rm_pend = find_matrix(addr);

	if(!rm_pend)
		return false;

	--matrix_count;
	
    memcpy(rm_pend, matrices+matrix_count, sizeof(struct _traced_matrix));
	
    return true;
}


void ssim_print_stats(void)
{
	VG_(printf)("\nSummary of marked matrices:\n");
	VG_(printf)("Cache holding %d bytes (%d lines, ass. %d, sets: %d).\n",
	            LINESIZE * CACHELINES, CACHELINES, SETSIZE, SETS);
	VG_(printf)("Misses:  stores %d / %d, loads %d / %d\n",
	            smisses, stores, lmisses, loads);
}
