/* Shared memory event bridge (Valgrind side)
 * Allows multiple, chunked ring buffers
 *
 * (C) 2011, Josef Weidendorfer
 */

#include "pub_tool_basics.h"
#include "pub_tool_vki.h"
#include "pub_tool_libcbase.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcproc.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_libcfile.h"
#include "pub_tool_aspacemgr.h"
#include "pub_tool_mallocfree.h"
#include "pub_tool_options.h"

#include "shm_vgprod.h"

#include <sys/wait.h>

/* to be exported by aspacemgr... */
extern SysRes VG_(am_shared_mmap_file_float_valgrind)
( SizeT length, UInt prot, Int fd, Off64T offset );

   
static char* shmaddr = 0;
static shm_header* shmh = 0;
static Int shmused;
static char shmfile[20];

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

/* statistics */
double attach_time;
double wait_time = 0.0;

char* shm_init(void)
{
    Char* magic = SHM_MAGIC;
    Char buf[4];
    SysRes res;
    int fd, i;

    if (shmaddr) return shmaddr;

    if (sizeof(shm_header) != 260)
      VG_(tool_panic)("SHM header size wrong.");

    VG_(sprintf)(shmfile,"/tmp/%s.%d", SHM_NAME, VG_(getpid)());
    res = VG_(open)(shmfile,
                   VKI_O_CREAT|VKI_O_RDWR|VKI_O_TRUNC,
		   VKI_S_IRUSR|VKI_S_IWUSR);
    if (sr_isError(res)) return 0;
    fd = (Int) sr_Res(res);
    VG_(lseek)(fd, SHMSIZE-1, VKI_SEEK_SET);
    buf[0] = 0;
    VG_(write)(fd, buf , 1);
    
    res = VG_(am_shared_mmap_file_float_valgrind)
       (SHMSIZE, VKI_PROT_READ|VKI_PROT_WRITE, fd, 0);
    VG_(close)(fd);

    if (sr_isError(res)) {
       VG_(unlink)(shmfile);
       return 0;
    }
    shmaddr = (char*) sr_Res(res);
    shmh = (shm_header*) shmaddr;

    for(i=0;i<8;i++)
      shmh->magic[i] = magic[i];
    shmh->size = SHMSIZE;
    shmh->producer_64bit = (sizeof(long) == 8);
    shmh->producer_initialized = 0;
    shmh->consumer_attached = 0;
    for(i=0;i<15;i++)
      shmh->seg[i].offset = 0;

    shmused = 256;

    if (VG_(clo_verbosity) >1)
      VG_(dmsg)("Event producer: created '%s', size %d.\n", shmfile, SHMSIZE);

    attach_time = wtime();

    return shmaddr;
}

void shm_startconsumer(char* exe, int start_consumer)
{
    Char pidstr[10];

    VG_(sprintf)(pidstr, "%d", VG_(getpid)());
    if (start_consumer) {
        if (VG_(fork)() == 0) {	    
            Char* argv[3] = { exe, pidstr, 0};
            Char* argv_verb[4] = { exe, "-v", pidstr, 0};
	    if (VG_(clo_verbosity) >1)
	      VG_(execv)(exe, argv_verb);
	    else
	      VG_(execv)(exe, argv);
	    VG_(dmsg)("ERROR: Can not run consumer '%s'.\n", exe);
	    VG_(dmsg)("       Run manually with '%s %s'.\n", exe, pidstr);
            VG_(exit)(1);
        }
    }
    else
        VG_(dmsg)("Run '%s %s' to start event consumer\n", exe, pidstr);
}

void shm_finish(void)
{
    if (!shmaddr) return;

    VG_(am_munmap_valgrind)( (Addr)shmaddr, SHMSIZE);
    shmaddr = 0;
    //VG_(unlink)(shmfile);
}

char* shm_alloc_segment(Char* name, Int size)
{
    char* res;
    int s, i;

    if (!shmh) return 0;

    size = (size | 63) +1;

    if (shmused + size >= SHMSIZE)
       VG_(tool_panic)("Out of SHM space.");

    for(s=0;s<15;s++)
      if (shmh->seg[s].offset == 0) break;
    if (s==15)
       VG_(tool_panic)("Out of SHM segment space.");

    shmh->seg[s].offset = shmused;
    shmh->seg[s].size = size;
    for(i=0; name[i] && (i<7); i++) shmh->seg[s].name[i] = name[i];
    for(; i<8; i++) shmh->seg[s].name[i] = 0;

    if (VG_(clo_verbosity) >1)
      VG_(dmsg)("Event producer: created seg '%s', size %d (seg# %d at %d).\n",
		name, size, s, shmused);

    res = shmaddr + shmused;
    shmused += size;
    return res;
}

/* shm is now initialized */
void shm_initialized(void)
{
    if (shmaddr)
	shmaddr[13] = 1;
}

#if 0
Int shm_wait(void)
{
  volatile Char* w;

  if (!shmaddr) return 0;
  shmaddr[13] = 1;

  VG_(printf)("Waiting for consumer to attach to SHM file '%s'...\n", shmfile);
  w = shmaddr+15;
  while(!*w);
  VG_(printf)("Consumer attached.\n");

  return 1;
}
#endif

shm_rb* shm_alloc_rb(Char* name, int count, int size)
{
  char* b;
  int s, i;
  shm_rb* rb;
  rb_header* h;

  s = ((size-1) | 63) +1;
  b = shm_alloc_segment(name, 64 + count * (64 + s));
  if (!b) return 0;

  rb = (shm_rb*) VG_(malloc)("shm_alloc_rb",
			     sizeof(shm_rb) + count * sizeof(rb_chunk));
  if (!rb) return 0; /* FIXME: free segment */

  h = (rb_header*) b;
  h->chunk_count = count;
  h->chunk_size = s;
  h->state0_offset = 64;
  h->buffer0_offset = (count+1) * 64;

  rb->header = h;
  rb->name = name;
  rb->fill_count = 0;
  rb->event_count = 0;
  rb->byte_count = 0;
  rb->first = &(rb->chunk[0]);
  for(i=0;i<count;i++) {
    b[64 + 64*i] = RBSTATE_EMPTY;

    rb->chunk[i].rb = rb;
    rb->chunk[i].state = & b[64 + 64*i];
    rb->chunk[i].buffer = & b[64*(count+1) + s * i];
    rb->chunk[i].size = size;
    rb->chunk[i].next = &(rb->chunk[ (i<count-1) ? i+1 : 0]);
  }

  return rb;
}

void shm_init_sending(rb_state* st, shm_rb* rb)
{
    rb_chunk* c = rb->first;
    st->current = c;

    tl_assert(*(c->state) == RBSTATE_EMPTY);

    st->write_ptr = c->buffer + 4;
    st->end_ptr = c->buffer + c->size;
    st->event_count = 0;

    if(0) VG_(printf)("Starting chunk at %p (offset 0x%x) with size %d bytes.\n",
                      c->buffer,
                      (int)(c->buffer - (unsigned char*) c->rb->header), c->size);
}


// called by start_event if buffer full
rb_chunk* next_chunk(rb_state* st)
{
  rb_chunk* c = st->current;
  int used = st->write_ptr - c->buffer;

  tl_assert(st->end_ptr == c->buffer + c->size);
  tl_assert2(used <= c->size,
             "Used %d > size %d", used, c->size);
  *(int*)(c->buffer) = used;
  if (*(c->state) != RBSTATE_EMPTY)
      VG_(tool_panic)("Filled non-empty chunck?");
  c->rb->fill_count++;
  c->rb->byte_count += used;
  c->rb->event_count += st->event_count;

  *(c->state) = RBSTATE_FULL;

  if(0) VG_(printf)("Filled chunk at %p (offset 0x%x) with %d bytes.\n",
                    c->buffer,
                    (int)(c->buffer - (unsigned char*) c->rb->header), used);

  c = c->next;
  if (*(c->state) != RBSTATE_EMPTY) {
      unsigned char* seg = (unsigned char*) c->rb->header;
      if(0) VG_(printf)("Waiting for chunk at 0x%x (state at 0x%x).\n",
			(int)(c->buffer - seg), (int)(c->state - seg));
      double t = wtime();
	  int i = 0;
      while(1) {
		if(*(c->state) == RBSTATE_EMPTY) break;
		if(i < 10000/*arbitrary*/); //sched_yield(); requires -lc which does weird things // VG_(vg_yield)(); <- I'd like to use that, but including pub_core_scheduler.h does weird things.
		else {
			i = 0;
			if(VG_(waitpid)(-1, NULL, WNOHANG|WEXITED) > 0) // child exited
				VG_(tool_panic)("unexpected child exit");
		}
		++i;
	  }
      wait_time += wtime() - t;
  }

  st->current = c;
  st->write_ptr = c->buffer + 4; // 4 bytes reserved for bytes used in chunk
  st->end_ptr = c->buffer + c->size;
  st->event_count = 0;

  if(0) VG_(printf)("Starting chunk at %p (offset 0x%x) with size %d bytes.\n",
                    c->buffer,
                    (int)(c->buffer - (unsigned char*) c->rb->header), c->size);

  return c;
}

void shm_close(rb_state* st)
{
    rb_chunk* c = st->current;
    int used = st->write_ptr - c->buffer;

    tl_assert(st->end_ptr == c->buffer + c->size);
    tl_assert2(used <= c->size,
               "Used %d > size %d", used, c->size);
    *(int*)(c->buffer) = used;
    if (*(c->state) != RBSTATE_EMPTY)
        VG_(tool_panic)("Filled non-empty chunck?");
    c->rb->fill_count++;
    c->rb->byte_count += used;
    c->rb->event_count += st->event_count;

    *(c->state) = RBSTATE_FULLEND;

    if(0) VG_(printf)("Filled last chunk at 0x%x with %d bytes.\n",
                      (int)(c->buffer - (unsigned char*) c->rb->header), used);

    if (VG_(clo_verbosity) <2) return;

    double t = wtime() - attach_time;
    double tt = t - wait_time;
    int chunks_produced = c->rb->fill_count;
    long events_produced = c->rb->event_count;
    long bytes_produced = c->rb->byte_count;
    double p = wait_time/t*100.0;
    double t2 = wait_time;
    double t3 = (double) events_produced / t / 1000000.0;
    double t4 = (double) bytes_produced / t / 1000000.0;
    double t5 = (double) events_produced / tt / 1000000.0;
    double t6 = (double) bytes_produced / tt / 1000000.0;
    VG_(dmsg)("Event producer (rb '%s') statistics:\n"
              "  total %d.%03ds (active %d.%03ds, waiting %d.%03ds = %d.%02d%%)\n"
              "  produced %d chunks, %ld events, %ld bytes\n"
              "  troughput %d.%03d MEv/s, %d.%03d MB/s (without waiting %d.%1d MEv/s, %d.%1d MB/s)\n",
              c->rb->name,
              (int) t, (int)(1000.0 * (t - (int)t)),
              (int) tt, (int)(1000.0 * (tt - (int)tt)),
              (int) t2, (int)(1000.0 * (t2 - (int)t2)),
              (int) p, (int)(100.0 * (p - (int)p)),
              chunks_produced, events_produced, bytes_produced,
              (int) t3, (int)(1000.0 * (t3 - (int)t3)),
              (int) t4, (int)(1000.0 * (t4 - (int)t4)),
              (int) t5, (int)(10.0 * (t5 - (int)t5)),
              (int) t6, (int)(10.0 * (t6 - (int)t6)));
}
