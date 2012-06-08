/*
 * Event consumer for mctracer: simple cache simulator.
 * For ETI @ TUM, (C) 2011 Josef Weidendorfer
 */

#include "simplesim.h"

#define _GNU_SOURCE
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

typedef struct _matrix_access_method {
    /* relative row number */
    int offset_m;
    /* relative column number */
    int offset_n;
    int misses;
    int hits;
} matrix_access_method;

typedef struct _matrix_access_data {
    /* last address accessed by this matrix */
    Addr last_accessed_addr;
    /* list of access methods used to retrieve/write data from/to the matrix */
    matrix_access_method access_methods[MAX_MATRIX_ACCESS_METHODS];
    int access_methods_count;
} matrix_access_data;

typedef struct _traced_matrix {
    /* start address of the matrix in memory */
    Addr start; 
    /* last address of the matrix in memory */
	Addr end;
    /* name of the matrix */
    char* name;
    /* size of each element of the matrix in bytes */
    int ele_size;
    /* number of rows */
    int m;
    /* number of columns */
    int n;
    /* contains data about the memory and cache access (patterns) */
    matrix_access_data access_data;
} traced_matrix;

traced_matrix traced_matrices [MAX_MATRIX_COUNT]; 

// number of traced matrices 
int traced_matrices_count = 0;

// if you want, you can implement something binary-searchish for this. The table is probably going to have 3 elements, though...
static traced_matrix* find_matrix(Addr access)
{
	traced_matrix* it  = traced_matrices;
	traced_matrix* end = traced_matrices + traced_matrices_count;

	for(; it < end; ++it) {
		if(it->start <= access && access < it->end) {
		    return it;
        }
    }

	return 0;
}

static void update_matrix_stats(Addr addr, SizeT size) {
    traced_matrix* matr;
	if((matr = find_matrix(addr)) != 0) {
        // ignore the first access
        if ((*matr).access_data.last_accessed_addr != -1) {
            int tmp;
           /* 
            VG_(printf)("################## NEW LOAD #################\n");
            VG_(printf)("start_addr=%d\n", (*matr).start);
            VG_(printf)("last_accessed_addr=%d\n", (*matr).access_data.last_accessed_addr);
            VG_(printf)("addr=%d\n", addr);
            VG_(printf)("m = %d\n", (*matr).m);
            VG_(printf)("ele_size=%d\n", (*matr).ele_size); 
*/
            // transform the last_accessed_addr to (m,n) representation 
            tmp  = ((*matr).access_data.last_accessed_addr - (*matr).start) / (*matr).ele_size;
            int last_n = tmp % (*matr).m;
            int last_m = (tmp - last_n) / (*matr).m;

            // transform addr to (m,n) representation
            tmp = (addr - (*matr).start) / (*matr).ele_size;
            int n = tmp % (*matr).m;
            int m = (tmp - n) / (*matr).m;

            // calculate the current access method
            int offset_n = n - last_n;
            int offset_m = m - last_m;
/*
            VG_(printf)("offset_n=%d\n", offset_n);
            VG_(printf)("offset_m=%d\n", offset_m);
*/
            int i;
            int amc = (*matr).access_data.access_methods_count;
            int is_hit = cache_ref(addr, size);
            bool found = false;
            
            // look for the access method and increase the hit/miss count
            for (i=0; i < amc; ++i) {
                matrix_access_method* am = (*matr).access_data.access_methods+i;
                
                if (am->offset_m == offset_m && am->offset_n == offset_n) {
                    // we got a matching access method
                    // ... now check for hit or miss
                    if (is_hit) {
                        am->hits++;
                    } else {
                        am->misses++;
                    }

                    found = true;
                    break;              
                }
            }

            // if no access method was found => add a new one
            if (!found) {
                matrix_access_method* am = ((*matr).access_data.access_methods + (*matr).access_data.access_methods_count);
                am->offset_m = offset_m;
                am->offset_n = offset_n;
                
                if (is_hit) {
                    // it's a hit
                    am->hits = 1;
                    am->misses = 0;
                } else {
                    // it's a miss
                    am->misses = 1;
                    am->hits = 0;
                }

                (*matr).access_data.access_methods_count++;

                if ((*matr).access_data.access_methods_count >= MAX_MATRIX_ACCESS_METHODS) {
                    VG_(tool_panic)("Max. number of access methods exceeded.");
                }
            } 
        }

        // update
        (*matr).access_data.last_accessed_addr = addr;
    } 
}

VG_REGPARM(2) void ssim_load(Addr addr, SizeT size)
{
    update_matrix_stats(addr,size);
}

VG_REGPARM(2) void ssim_store(Addr addr, SizeT size)
{
    update_matrix_stats(addr,size);
}

bool ssim_matrix_tracing_start(Addr addr, int m, int n, int ele_size, char* name)
{
	if(traced_matrices_count >= MAX_MATRIX_COUNT) {
		char msg[100];
        VG_(sprintf)(msg, "Max. number of matrices exceeded. Can't track more than %d matrices", MAX_MATRIX_COUNT);
        VG_(tool_panic)(msg);

        return false;
    }

    /* store general information about the matrix */
    (traced_matrices + traced_matrices_count)->start = addr;
	(traced_matrices + traced_matrices_count)->end = addr + m*n*ele_size;
	(traced_matrices + traced_matrices_count)->name = name;
    // size of each element of the matrix in bytes
    (traced_matrices + traced_matrices_count)->ele_size = ele_size;
    (traced_matrices + traced_matrices_count)->m = m;
    (traced_matrices + traced_matrices_count)->n = n;

    /* initialize memory access infos */
    (traced_matrices + traced_matrices_count)->access_data.last_accessed_addr = -1;
    // allocate space for sqrt(N) access methods, but at least 8
    //size_t acc_mths = 20;//max(sqrt(max(m,n)), 8);
    //(traced_matrices + traced_matrices_count)->access_data.access_methods = VG_(calloc)("matrix_access_methods", acc_mths, sizeof(struct _matrix_access_method));
    
    ++traced_matrices_count;

    return true;
}

bool ssim_matrix_tracing_stop(Addr addr)
{
	traced_matrix* rm_pend = find_matrix(addr);

	if(!rm_pend)
		return false;

	//--traced_matrices_count;
	
    //memcpy(rm_pend, traced_matrices+traced_matrices_count, sizeof(struct _traced_matrix));
	
    return true;
}

void ssim_print_stats(void)
{
    int i;
    for (i=0; i<traced_matrices_count; ++i) {
        traced_matrix* tm = traced_matrices + i;
        VG_(printf)("Name: %s\nStart Address: %d\nElement Size: %d\n", tm->name, tm->start, tm->ele_size);
        VG_(printf)("Rows: %d\nColumns: %d\n\n", tm->m, tm->n);
        
        int j;
        for (j = 0; j < tm->access_data.access_methods_count; ++j) {
            matrix_access_method* am = tm->access_data.access_methods+j;
            VG_(printf)("%d. Access Method (%d,%d)\n", j+1, am->offset_m, am->offset_n);
            VG_(printf)("Hits: %d\nMisses: %d\n", am->hits, am->misses);
        }

        VG_(printf)("\n\n");
    }
}
