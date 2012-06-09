/*
 * Event consumer for mctracer: simple cache simulator.
 * For ETI @ TUM, (C) 2011 Josef Weidendorfer
 */

#include "simplesim.h"

#define _GNU_SOURCE
#include <stdbool.h>
#include <stdio.h>
#include "pub_tool_mallocfree.h" // VG_(malloc)
#include "pub_tool_libcprint.h" // VG_(printf)
#include "pub_tool_libcassert.h" // VG_(tool_panic)

#define MATRIX_LOAD 'L'
#define MATRIX_STORE 'S'

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

typedef struct _matrix_hit_and_miss {
    int hits;
    int misses;
} matrix_access_count;

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
    /* number of hits/misses per element for loads */
    matrix_access_count* load_count;
    /* number of hits/misses per element for stores */
    matrix_access_count* store_count;
} traced_matrix;

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

	for(i=0; i < tmax; ++i) {
        if (traced_matrices_index[i]->start <= access &&
            traced_matrices_index[i]->end > access) {
            return traced_matrices_index[i];
        }
    }

	return 0;
}

static void update_matrix_access_stats(traced_matrix* matr, int is_hit, Addr addr, SizeT size) {
	if(matr) {
        // ignore the first access
        if ((*matr).access_data.last_accessed_addr != -1) {
            int tmp;

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

            int i;
            int amc = (*matr).access_data.access_methods_count;
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

static void update_matrix_stats(Addr addr, SizeT size, char type) {
    traced_matrix* matr = find_matrix(addr);
    
    if (matr) {
        int is_hit = cache_ref(addr, size);
        // update access method stats
        update_matrix_access_stats(matr, is_hit, addr, size);
        
        // update general access stats
        int offset = (addr - matr->start) / matr->ele_size;
        if (is_hit) {
            if (type == MATRIX_LOAD) {
                matr->load_count[offset].hits++;
            } else {
                matr->store_count[offset].hits++;
            }
        } else {
            if (type == MATRIX_LOAD) {
                matr->load_count[offset].misses++;
            } else {
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

bool ssim_matrix_tracing_start(Addr addr, int m, int n, int ele_size, char* name)
{
	if(traced_matrices_count >= MAX_MATRIX_COUNT) {
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
    matr->access_data.last_accessed_addr = -1;
    matr->load_count = (matrix_access_count*) VG_(malloc)("matrix load count", m*n*sizeof(matrix_access_count));
    matr->store_count = (matrix_access_count*) VG_(malloc)("matrix store count", m*n*sizeof(matrix_access_count));

    // initialize every element with 0
    int i, j, offset;
    for(i = 0; i < m; ++i) {
        for(j = 0; j < n; ++j) {
            offset = i*n + j;
            
            matr->load_count[offset].hits = 0;
            matr->load_count[offset].misses = 0;

            matr->store_count[offset].hits = 0;
            matr->store_count[offset].misses = 0;
        }
    }
    
    /* Update the matrix index */

    if (traced_matrices_count == 1 || stopped_matrices_count == 0) {
        traced_matrices_index[traced_matrices_count] = matr;
    } else {
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

	if(!matr) {
		return false;
    }

    /* We'll store "untraced" matrices at 
     * the end of the array, so that we'll find the
     * actively traced ones faster.
     */
 
    // the last actively traced matrix in the array
    int last_traced_mtr_offset = traced_matrices_count - stopped_matrices_count - 1;
    traced_matrix* tmp_tm = traced_matrices_index[last_traced_mtr_offset];

    // look for the mtr from 0 to last_traced_mtr_offset-1, so
    // that we don't swap a matrix with itself
    int i;
    for (i=0; i < last_traced_mtr_offset-1; ++i) {
        if (traced_matrices_index[i]->start == matr->start) {
            traced_matrices_index[last_traced_mtr_offset] = matr;
            traced_matrices_index[i] = tmp_tm;
        }
    }

    stopped_matrices_count++;
    return true;
}

void ssim_save_stats(char* fname)
{
    FILE* fh = fopen(fname, "w+");
    
    fputc(0xAF, fh);
    fputc(0xFE, fh);

    fclose(fh);
}
