/*
 * Event consumer for mctracer: simple cache simulator.
 * For ETI @ TUM, (C) 2011 Josef Weidendorfer
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

#include "shmlib/shm_consumer.h"

// 64-bit type for addresses: this needs mctracer to be 64bit binary !!
typedef unsigned long long Addr;

// type Addr is used in events definitions
#include "tr_shmevents.h"

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

typedef struct _cacheline {
	Addr tag;            // 64 bit architecture
} Cacheline;

Cacheline cache[CACHELINES];

void cache_clear()
{
	int i;

	for(i=0; i<CACHELINES; i++) {
		cache[i].tag = 0;
	}
}

// a reference into a set of the cache, return 1 on hit
int cache_setref(int set_no, Addr tag)
{
	int i, j;
	Cacheline* set = cache + set_no * SETSIZE;
	unsigned old_mask;

	/* Test all lines in the set for a tag match
	 * If the tag is another than the MRU, move it into the MRU spot
	 * and shuffle the rest down.
	 */
	for (i = 0; i < SETSIZE; i++) {
		if (tag == set[i].tag) {
			for (j = i; j > 0; j--) {
				set[j].tag = set[j - 1].tag;
			}
			set[0].tag = tag;

			return 1;
		}
	}

	/* A miss;  install this tag as MRU, shuffle rest down. */
	for (j = SETSIZE - 1; j > 0; j--) {
		set[j].tag = set[j - 1].tag;
	}
	set[0].tag = tag;

	return 0;
}

// a reference at address <a> with size <s>, return 1 on hit
int cache_ref(Addr a, int size)
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

/* thread executing next memory accesses */
int tid = 0;


void run_tid(ev_run_tid* e)
{  
	// not really needed here: we assume a shared cache for all threads
	tid = e->tid;
}

void data_read(ev_data_read* e)
{
	int res;
	res = cache_ref(e->addr, e->len);
	printf(" > Load  by T%d at %p, size %2d: %s\n",
		 tid, (void*) e->addr, e->len, res ? "Hit ":"Miss");
	loads++;
	if (res == 0) lmisses++;
}

void data_write(ev_data_write* e)
{
	int res;
	res = cache_ref(e->addr, e->len);
	printf(" > Store by T%d at %p, size %2d: %s\n",
		 tid, (void*) e->addr, e->len, res ? "Hit ":"Miss");
	stores++;
	if (res == 0) smisses++;
}

int main(int argc, char* argv[])
{
	shm_buf* buf;
	shm_rb* rb;
	rb_chunk* chunk;
	tr_event* e;

	/* initialize event passing via shared memory */
	buf = shm_init(argc, argv);
	rb = open_rb(buf, "tr_main");
	if (!rb) {
		printf("Cannot open ring buffer 'tr_main'\n");
		exit(1);
	}

	cache_clear();

	chunk = open_first(rb);
	while( (e = (tr_event*) next_event(&chunk)) ) {
		switch(e->tag) {
			case TR_RUN_TID:
				run_tid(&(e->run_tid));
				break;
			case TR_DATA_READ:
				data_read(&(e->data_read));
				break;
			case TR_DATA_WRITE:
				data_write(&(e->data_write));
				break;
			default:
				printf(" Unknown event tag %d\n", e->tag);
				abort();
				break;
		}
	}

	printf("\nSummary:\n");
	printf("Cache holding %d bytes (%d lines, ass. %d, sets: %d).\n",
			LINESIZE * CACHELINES, CACHELINES, SETSIZE, SETS);
	printf("Misses:  stores %d / %d, loads %d / %d\n",
			smisses, stores, lmisses, loads);
	return 1;
}
