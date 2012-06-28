/*
 * Event consumer for mctracer: simple cache simulator.
 * For ETI @ TUM, (C) 2011 Josef Weidendorfer
 */

/**
 * TODO: replace C datatypes with the corresponding valgrind types
 *
 */

#include "simplesim.h"

#define _GNU_SOURCE
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include "pub_tool_libcbase.h" // VG_(strlen), VG_(memset)
#include "pub_tool_libcfile.h" // VG_(open), VG_(write)
#include "pub_tool_vki.h" // VKI_O_TRUNC ... 
#include "pub_tool_mallocfree.h" // VG_(malloc)
#include "pub_tool_libcprint.h" // VG_(printf)
#include "pub_tool_libcassert.h" // VG_(tool_panic)

/* ----------------------------------------------------------------*/

/**
 * utitlity functions, because of the missing libc
 */
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

static void mark_pattern_findings(traced_matrix * matr, access_pattern * const ap, bool * const patterned_access)
{
	// convenience shorthands
	const unsigned int count = matr->access_event_count;
	access_event * const accbuf = matr->access_buffer;

	if(ap->length == 0) 
		return;
	unsigned int j;
	for(j = 0; j < count; ++j) // loop over access buffer, loop variable is modified inside of the loop
	{
		if(ap->length + j >= count) // can't match pattern if it's longer than the remaining accesses
			break;
			
		unsigned int apstep;
		for(apstep = 0; apstep < ap->length; ++apstep) // loop over access method steps
			if(ap->steps[apstep].offset_m != accbuf[j+apstep].offset.m ||
			   ap->steps[apstep].offset_n != accbuf[j+apstep].offset.n)
				break;
		if(apstep == ap->length) // means that all were equal
		{
			unsigned int k;
			for(k = 0; k < ap->length; ++k)
			{
				if(accbuf[j+k].is_hit)
					++ap->steps[k].hits;
				else
					++ap->steps[k].misses;
				patterned_access[j+k] = true;
			}
			j += ap->length - 1; // -1 to counter loop increment
			++(ap->occurences);
		}
	}
}

static void process_pattern_buffer(traced_matrix * matr)
{
	// convenience shorthands
	const unsigned int count = matr->access_event_count;
	access_event * const accbuf = matr->access_buffer;

	// buffer for registering that an access has been included in an existing pattern
	bool patterned_access[MATRIX_ACCESS_ANALYSIS_BUFFER_LENGHT];
	VG_(memset)(patterned_access, false, MATRIX_ACCESS_ANALYSIS_BUFFER_LENGHT);

	// go over all the existing access patterns and find matching accesses
	unsigned int i;
	for(i = 0; i < MAX_PATTERNS_PER_MATRIX; ++i) // loop over access patterns
		mark_pattern_findings(matr, matr->access_patterns + i, patterned_access);
	// find new patterns
	for(i = 0; i < count - MAX_PATTERN_LENGTH*2; ++i)
	{
		if(patterned_access[i])
			continue;
		// single access repetition locating
		unsigned int length;
		for(length = 1; length <= MAX_PATTERN_LENGTH; ++length)
			if(accbuf[i].offset.n == accbuf[i+length].offset.n &&
			   accbuf[i].offset.m == accbuf[i+length].offset.m)
				break;
		if(length > MAX_PATTERN_LENGTH)
			continue;
		unsigned int j;
		// sequence equality check
		for(j = 1; j < length; ++j)
			if(accbuf[i+j].offset.n != accbuf[i+length+j].offset.n ||
			   accbuf[i+j].offset.m != accbuf[i+length+j].offset.m)
				break;
		if(j < length)
			continue;
		// found a pattern, find a place to store it in.
		// (Keeping a list of all located patterns around would require a much more sofisticated pattern recognition algorithm
		//  it would probably be slower, too.)
		unsigned int matr_accesses = matr->loads.misses + matr->loads.hits + matr->stores.misses + matr->stores.hits;
		float max_expendability = 0;
		access_pattern * rap = 0; // replaced access pattern
		for(j = 0; j < MAX_PATTERNS_PER_MATRIX; ++j)
		{
			access_pattern * const ap = matr->access_patterns + j;
			if(ap->length == 0)
			{
				rap = ap;
				break;
			}
			// the relevance of a pattern is computed by the fraction of accesses that match that pattern since that pattern emerged.
			float expendability = (double)(matr_accesses - ap->accesses_before_lifetime + 1000) / (ap->occurences * ap->length + 1000); // the constant summands prevent instabilities with small numbers
			if(expendability > max_expendability)
			{
				rap = ap;
				max_expendability = expendability;
			}
		}
		if(rap->steps)
			VG_(free)(rap->steps);
		rap->steps = VG_(malloc)("access pattern",length*sizeof(access_pattern));
		for(j = 0; j < length; ++j)
		{
			rap->steps[j].offset_n = accbuf[i+j].offset.n;
			rap->steps[j].offset_m = accbuf[i+j].offset.m;
			rap->steps[j].hits = 0;
			rap->steps[j].misses = 0;
		}
		rap->length = length;
		rap->occurences = 0;
		rap->accesses_before_lifetime = matr_accesses;
		mark_pattern_findings(matr, rap, patterned_access);
	}
	// consistency check: eleminate patterns which are subpatterns to others
	for(i = 0; i < MAX_PATTERNS_PER_MATRIX; ++i) // loop over patterns which may be eliminated
	{
		access_pattern * const oap = matr->access_patterns + i; // outer access pattern
		if(oap->length == 0)
			continue;
		unsigned int j;
		for(j = 0; j < MAX_PATTERNS_PER_MATRIX; ++j) // loop over patterns which could eliminate oap
		{
			access_pattern * const iap = matr->access_patterns + j; // inner access pattern	
			if(iap->length < oap->length)
				continue;
			unsigned int k;
			for(k = 0; k < iap->length; ++k)
			{
				unsigned int l;
				for(l = 0; l < oap->length; ++l)
					if(oap->steps[l].offset_n != iap->steps[k].offset_n ||
					   oap->steps[l].offset_m != iap->steps[k].offset_m)
						break;
				if(l == oap->length)
				{
					if(oap->steps == 0)
						VG_(tool_panic)("inconsistent state of patterns");
					VG_(free)(oap->steps);
					VG_(memset)(oap, 0, sizeof(access_pattern));
					goto outer_matrix_eliminated;
				}
			}
		}
		outer_matrix_eliminated: continue; // that continue prevents a compilation error
	}
	// preserve the last few accesses which can not be accounted to maximum pattern lengths
	VG_(memmove)(accbuf, accbuf + count - MAX_PATTERN_LENGTH, MAX_PATTERN_LENGTH);
	matr->access_event_count = MAX_PATTERN_LENGTH;
}

static void update_matrix_pattern_stats(traced_matrix * matr, short offset_n, short offset_m, bool is_hit)
{
	if(!matr || !matr->access_buffer)
		VG_(tool_panic)("internal error: matrix passed for pattern finding without access buffer.");
	// store new event
	access_event ev;
	ev.is_hit = is_hit;
	ev.offset.n = offset_n;
	ev.offset.m = offset_m;
	matr->access_buffer[matr->access_event_count] = ev;
	if(++matr->access_event_count == MATRIX_ACCESS_ANALYSIS_BUFFER_LENGHT)
		process_pattern_buffer(matr);
}

static void update_matrix_access_stats(traced_matrix* matr, matrix_access_data* access_data, int is_hit, Addr addr, SizeT size)
{
	if(matr && access_data)
	{
		// transform addr to (m,n) representation
		Addr tmp = (addr - (*matr).start) / (*matr).ele_size;
		unsigned short n = tmp % (*matr).m;
		unsigned short m = (tmp - n) / (*matr).m;

		// ignore the first access
		if (access_data->last_access.m != SHRT_MIN && access_data->last_access.n != SHRT_MIN)
		{

			// calculate the current access method
			short offset_n = n - access_data->last_access.n;
			short offset_m = m - access_data->last_access.m;

			int i;
			int amc = access_data->access_methods_count;
			bool found = false;

			// look for the access method and increase the hit/miss count
			for (i=0; i < amc; ++i)
			{
				matrix_access_method* am = access_data->access_methods+i;

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
				matrix_access_method* am = (access_data->access_methods + access_data->access_methods_count);
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

				access_data->access_methods_count++;

				if (access_data->access_methods_count >= MAX_MATRIX_ACCESS_METHODS)
				{
					VG_(tool_panic)("Max. number of access methods exceeded.");
				}
			}
			update_matrix_pattern_stats(matr, offset_n, offset_m, is_hit);
		}

		// update
		access_data->last_access.n = n;
		access_data->last_access.m = m;

	}
}

static void update_matrix_stats(Addr addr, SizeT size, char type)
{
	traced_matrix* matr = find_matrix(addr);

	if (matr)
	{
		int is_hit = cache_ref(addr, size);
		int offset = (addr - matr->start) / matr->ele_size;

        if (type == MATRIX_LOAD) {
            //update access method stats
            update_matrix_access_stats(matr, &(matr->load_access_data), is_hit, addr, size);

            // update general access stats
            if (is_hit) {
                matr->load_count[offset].hits++;
                matr->loads.hits++;
            } else {
                matr->load_count[offset].misses++;
                matr->loads.misses++;
            }
        } else {
            update_matrix_access_stats(matr, &(matr->store_access_data), is_hit, addr, size);

            if (is_hit) {
                matr->store_count[offset].hits++;
                matr->stores.hits++; 
            } else {
                matr->store_count[offset].misses++;
                matr->stores.misses++;
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
	
    matr->name = (char*) VG_(malloc)("name", VG_(strlen)(name)*sizeof(char));
    VG_(strcpy)(matr->name, name);

	// size of each element of the matrix in bytes
	matr->ele_size = ele_size;
	matr->m = m;
	matr->n = n;

	/* initialize memory access infos */
	matr->load_access_data.last_access.n = SHRT_MIN;
	matr->load_access_data.last_access.m = SHRT_MIN;
	matr->store_access_data.last_access.n = SHRT_MIN;
	matr->store_access_data.last_access.m = SHRT_MIN;
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
	matr->access_buffer = (access_event*) VG_(malloc)("matrix access event buffer", MATRIX_ACCESS_ANALYSIS_BUFFER_LENGHT*sizeof(access_event));
	matr->access_event_count = 0;
	VG_(memset)(matr->access_patterns, 0, MAX_PATTERNS_PER_MATRIX*sizeof(access_pattern));

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

	// add remaining events to sequences
	process_pattern_buffer(matr);

	/* We'll store "untraced" matrices at
	 * the end of the array, so that we'll find the
	 _* actively traced ones faster.
	 */

	// the last actively traced matrix in the array
	int last_traced_mtr_offset = traced_matrices_count - stopped_matrices_count - 1;
	traced_matrix* tmp_tm = traced_matrices_index[last_traced_mtr_offset];

	// free unused memory
	VG_(free)(matr->access_buffer);
	matr->access_buffer = NULL;

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

static void write_access_methods(Int fd, matrix_access_data* access_data) {
    /**
	 * Access methods (ZA) (2*N*12 bytes)
	 */

	int accm_count = access_data->access_methods_count;

	if (accm_count > 8)
    {
		// sort the access methods according to their cumulative hits and misses
		ssim_qsort(access_data->access_methods, 0, access_data->access_methods_count);
	}

	accm_count = accm_count > 8 ? 8 : accm_count;

	int k;
	for (k = 0; k < accm_count; ++k)
	{
		matrix_access_method accm = access_data->access_methods[k];

		// ZA:OM
		VG_(write)(fd, &(accm.offset_m), sizeof(ushort));

		// ZA:ON
		VG_(write)(fd, &(accm.offset_n), sizeof(ushort));

		// ZA:AH
		VG_(write)(fd, &(accm.hits), sizeof(unsigned int));

		// ZA:AM
		VG_(write)(fd, &(accm.misses), sizeof(unsigned int));
	}
}

void ssim_save_stats(HChar* fname)
{
	SysRes fd_res = VG_(open)(fname, VKI_O_WRONLY|VKI_O_TRUNC|VKI_O_CREAT, VKI_S_IRUSR|VKI_S_IWUSR|VKI_S_IRGRP|VKI_S_IWGRP);
    Int fd = sr_Res(fd_res);

    // no SESE for julius
    if (fd == -1) {
        char msg[100];
        VG_(sprintf)(msg, "Failed to open the file '%s' for writing.", fname);
        VG_(tool_panic)(msg);
        return;
    }

	/**
	 * FILE HEADER (4 bytes)
	 */

    // magic number
    byte magic_num[2] = {0xAF, 0xFE};
    // version of the file format
    byte version = 0x2;
    byte matr_count = (byte) traced_matrices_count;

	VG_(write)(fd, magic_num, 2*sizeof(byte));
	VG_(write)(fd, &version, sizeof(byte));
	VG_(write)(fd, &matr_count, sizeof(byte));

	/**
	 * MATRICES HEADER (N*34 bytes)
	 */

	// MH:MIBADR => initalize with the DH and MH size
	unsigned int mibaddr = 4 + traced_matrices_count*34;
	int i;
	for (i = 0; i < traced_matrices_count; ++i)
	{
		// MH:GM (2 bytes)
		VG_(write)(fd, &(traced_matrices[i].m), sizeof(ushort));

		// MH:GN (2 bytes)
		VG_(write)(fd, &(traced_matrices[i].n), sizeof(ushort));

		// MH:AZL (1 byte), limited to 8
		int tmp = traced_matrices[i].load_access_data.access_methods_count;
		byte accm_loads_count = tmp < 8 ? (byte)tmp : 8;
		VG_(write)(fd, &accm_loads_count, sizeof(byte));

        // MH:AZS (1 byte), limited to 8
		tmp = traced_matrices[i].store_access_data.access_methods_count;
		byte accm_stores_count = tmp < 8 ? (byte)tmp : 8;
		VG_(write)(fd, &accm_stores_count, sizeof(byte));

		// MH:ADR (8 byte)
		VG_(write)(fd, &(traced_matrices[i].start), sizeof(Addr));

		// MH:MIBADR (4 byte)
		VG_(write)(fd, &mibaddr, sizeof(unsigned int));
    
        // MH:ALH (4 byte)
        VG_(write)(fd, &(traced_matrices[i].loads.hits), sizeof(unsigned int));
        
        // MH:ALM (4 byte)
        VG_(write)(fd, &(traced_matrices[i].loads.misses), sizeof(unsigned int));
        
        // MH:ASH (4 byte)
        VG_(write)(fd, &(traced_matrices[i].stores.hits), sizeof(unsigned int));
        
        // MH:ASM (4 byte)
        VG_(write)(fd, &(traced_matrices[i].stores.misses), sizeof(unsigned int));

		// calculate the address of the next MIB
		// matrices
		mibaddr += 2 * traced_matrices[i].m * traced_matrices[i].n;
		// access methods
		mibaddr += 12 * (accm_loads_count + accm_stores_count);
		// name + \0
		mibaddr += VG_(strlen)(traced_matrices[i].name) + 1;
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

		VG_(write)(fd, loads_array, rows*cols);
		VG_(write)(fd, stores_array, rows*cols);

		// free all that memory
		VG_(free)(loads_array);
		VG_(free)(stores_array);
		VG_(free)(traced_matrices[i].load_count);
		VG_(free)(traced_matrices[i].store_count);
		for(l = 0; l < MAX_PATTERNS_PER_MATRIX; ++l)
			VG_(free)(traced_matrices[i].access_patterns[l].steps);

        // LOADS
        write_access_methods(fd, &(traced_matrices[i].load_access_data));
        // STORES
        write_access_methods(fd, &(traced_matrices[i].store_access_data));

		/**
		 * Name (NA)
		 */

		// matrix name + \0
		VG_(write)(fd, traced_matrices[i].name, VG_(strlen)(traced_matrices[i].name)+1);
        VG_(free)(traced_matrices[i].name);
	}

	VG_(close)(fd);
}
