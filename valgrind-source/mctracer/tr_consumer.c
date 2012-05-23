/*
 * Event consumer for mctracer: simple event dump to stdout.
 * For ETI @ TUM, (C) 2011 Josef Weidendorfer
 */

#include "shm_consumer.h"

/* hack: only works when event producer uses same architecture */
typedef unsigned long Addr;

#include "tr_shmevents.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void run_tid(ev_run_tid* e)
{
  printf("Thread %d\n", e->tid);
}

void data_read(ev_data_read* e)
{
  printf("Data Read %p,%d\n", e->addr, e->len);
}

void data_write(ev_data_write* e)
{
  printf("Data Write %p,%d\n", e->addr, e->len);
}

int main(int argc, char* argv[])
{
    shm_buf* buf;
    shm_rb* rb;
    rb_chunk* chunk;
    tr_event* e;

    buf = shm_init(argc, argv);
    rb = open_rb(buf, "tr_main");
    if (!rb) {
      printf("Cannot open ring buffer 'tr_main'\n");
      exit(1);
    }

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

    return 0;
}
