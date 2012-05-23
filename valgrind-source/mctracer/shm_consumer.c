/* Shared memory event bridge (consumer side)
 * Allows multiple, chunked ring buffers
 *
 * (C) 2011, Josef Weidendorfer
 */

#include "shm_consumer.h"
#include "shm_common.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <assert.h>
#include <stdarg.h>

struct _shm_buf {
    shm_header* h;
    char file[20];
};

struct _rb_chunk {
  shm_rb* rb;
  volatile unsigned char* state;
  unsigned char* buffer;
  rb_chunk* next;

  int used; /* >0 if reading the chunk */
  int read; /* read pointer */
};

struct _shm_rb {
  rb_header* header;
  rb_chunk* first;
  rb_chunk chunk[0];
};

/*--------------------------------------------------------------
 * Time measurement helpers
 */

double wtime(void);

/* enables use of rdtsc, assume 2.4 GHz */
#define TSCRATE 2400

static inline
unsigned long long rdtsc_read(void)
{
   unsigned long long val;

#ifdef __amd64__
   /* x86 64bit specific */
   unsigned int _hi,_lo;
   asm volatile("rdtsc":"=a"(_lo),"=d"(_hi));
   val = ((unsigned long long int)_hi << 32) | _lo;
#else
   /* x86 32bit specific */
   asm volatile("rdtsc" : "=A" (val));
#endif
   return val;
}

double wtime(void)
{
#ifdef TSCRATE
   /* Use tsc counter */
   return (double) rdtsc_read() / (double) TSCRATE / 1000000.0;
#else
    struct timeval tv;
    double res = 0.0;

    gettimeofday(&tv, 0);
    res = (double) tv.tv_sec;
    res += (double) tv.tv_usec / 1000000.0;

    return res;
#endif
}

/*--------------------------------------------------------------
 * Event bridge functions
 */

#define VERBOSE 0

/* statistics */
static double attach_time;
static unsigned chunks_consumed = 0;
static unsigned long long events_consumed = 0;
static unsigned long long bytes_consumed = 0;
static double wait_time = 0.0;

static int producer_pid = 0;
static int verbose = 0;

shm_buf* attach(int pid)
{
    int fd;
    shm_buf* b;
    void* addr;

    b = (shm_buf*) malloc(sizeof(shm_buf));
    if (!b) return 0;

    sprintf(b->file, "/tmp/%s.%d", SHM_NAME, pid);
    fd = open(b->file, O_RDWR);
    if (fd<0) {
	free(b);
	return 0;
    }

    addr = mmap(0, SHMSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == (void*)-1) {
	free(b);
	return 0;
    }
    close(fd);
    unlink(b->file);

    producer_pid = pid;

    b->h = (shm_header*) addr;
    shm_printf("Event consumer: attaching to '%s'.\n", b->file);
    if (b->h->producer_initialized == 0) {
	volatile char* w = &(b->h->producer_initialized);
	shm_printf("Waiting for producer to finish initialization...\n");
	while(!*w);
	shm_printf("Done.\n");
    }

    // check for same arch width in producer and consumer
    assert( b->h->producer_64bit ? (sizeof(long)==8) : (sizeof(long)==4));

    b->h->consumer_attached = 1;

    attach_time = wtime();

    return b;
}

shm_buf* shm_init(int argc, char* argv[])
{
  int pid = 0;
  shm_buf* b;
  int arg;

  for(arg=1; arg<argc; arg++) {
    if (argv[arg][0] == '-') {
      if (argv[arg][1] == 'v')
	verbose++;
    }
    else
      pid = atoi(argv[arg]);
  }

  if (pid==0) {
    printf("Usage: %s [-v] <pid>\n", argv[0]);
    exit(1);
  }

  b = attach(pid);
  return b;
}

void shm_printf(const char *format, ...)
{
    char myformat[512];
    va_list vargs;

    if (verbose == 0) return;
    va_start(vargs, format);
    sprintf(myformat, "++%d++ %s", producer_pid, format);
    vfprintf(stderr, myformat, vargs);
    va_end(vargs);
}

shm_rb* open_rb(shm_buf* b, char* name)
{
    int s, i;
    rb_header* h;
    shm_rb* rb;
    char* seg;

    if (!b) return 0;
    for(s=0;s<15;s++)
	if ((b->h->seg[s].offset >0) &&
	    (strcmp(name, b->h->seg[s].name)==0)) break;
    if (s==15) return 0;

    seg = (char*)(b->h) + b->h->seg[s].offset;
    h = (rb_header*) seg;

    rb = (shm_rb*) malloc(sizeof(shm_rb) + h->chunk_count * sizeof(rb_chunk));
    if (!rb) return 0;

    rb->header = h;
    rb->first = &(rb->chunk[0]);
    for(i=0;i<h->chunk_count;i++) {
      rb->chunk[i].rb = rb;
      rb->chunk[i].state = & seg[64 + 64*i];
      rb->chunk[i].buffer = & seg[64*(h->chunk_count+1) + h->chunk_size * i];
      rb->chunk[i].used = -1;
      rb->chunk[i].read = 0;
      rb->chunk[i].next = &(rb->chunk[ (i<h->chunk_count-1) ? i+1 : 0]);
    }

    shm_printf("Event consumer: seg '%s' (at 0x%x, size %d): ring with %d chunks a %d bytes.\n",
	   name, b->h->seg[s].offset, b->h->seg[s].size,
	   h->chunk_count, h->chunk_size);

    return rb;
}

static void open_chunk(rb_chunk** cPtr)
{
    rb_chunk* c = *cPtr;

    if (*(c->state) == RBSTATE_EMPTY) {
	double t;

#if VERBOSE
	unsigned char* seg = (char*) c->rb->header;

	printf("Waiting for chunk at 0x%x (state at 0x%x) to be filled.\n",
	       (int)(c->buffer - seg), (int)(c->state - seg));
#endif

	t = wtime();
	while(*(c->state) == RBSTATE_EMPTY) {}
	wait_time += wtime() - t;
    }

    c->used = *(int*)c->buffer;
    c->read = 4;

#if VERBOSE
    printf("Opened chunk at 0x%x for reading (has %d bytes).\n",
	   (int)(c->buffer - (char*) c->rb->header), c->used);
#endif

    assert(c->used <= c->rb->header->chunk_size);
}

rb_chunk* open_first(shm_rb* rb)
{
    rb_chunk* c = rb->first;
    open_chunk(&c);

    return c;
}

rb_chunk* finish_chunk(rb_chunk* c, rb_chunk** cPtr)
{
  assert(c->read == c->used);
  chunks_consumed++;
  bytes_consumed += c->read;
  if (*(c->state) == RBSTATE_FULLEND) {
    double t = wtime() - attach_time;
    double tt = t - wait_time;
    shm_printf("Event consumer: statistics\n");
    shm_printf("  run %.3f secs since attaching (%.3f secs waiting = %.2f%%)\n",
	       t, wait_time, wait_time/t*100.0);
    shm_printf("  consumed %d chunks, %lld events, %lld bytes\n",
	       chunks_consumed, events_consumed, bytes_consumed);
    shm_printf("  troughput %.3f MEv/s, %.3f MB/s (without waiting %.1f MEv/s, %.1f MB/s)\n",
	   (double) events_consumed / t / 1000000.0,
	   (double) bytes_consumed / t / 1000000.0,
	   (double) events_consumed / tt / 1000000.0,
	   (double) bytes_consumed / tt / 1000000.0 );
    return 0;
  }
  *(c->state) = RBSTATE_EMPTY;
  c = c->next;
  *cPtr = c;
  open_chunk(cPtr);

  return c;
}

unsigned char* next_event(rb_chunk** cPtr)
{
    rb_chunk* c = *cPtr;
    unsigned char* e;

    if (c->read >= c->used) {
      c = finish_chunk(c, cPtr);
      if (!c) return 0;
    }
    e = c->buffer + c->read;
    events_consumed++;

#if VERBOSE
    printf("Got event %d (len %d) at %d/%d of chunk at 0x%x.\n",
	   (int)e[1], (int)e[0], c->read, c->used,
	   (int)(c->buffer - (unsigned char*) c->rb->header));
#endif

    c->read += (int)e[0];
    return e;
}

