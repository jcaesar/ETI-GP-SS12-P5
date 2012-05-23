/* Shared memory event bridge (Valgrind side)
 * Allows multiple, chunked ring buffers
 *
 * (C) 2011, Josef Weidendorfer
 */

#ifndef SHM_VGPROD_H
#define SHM_VGPROD_H

#include "pub_tool_libcassert.h"

#include "shm_common.h"

#define MAX_EVENTLEN 252

typedef struct _rb_chunk rb_chunk;
typedef struct _rb_state rb_state;
typedef struct _shm_rb shm_rb;

struct _rb_chunk {
  shm_rb* rb;
  volatile unsigned char* state;
  unsigned char* buffer;
  rb_chunk* next;
  int size;
};

struct _shm_rb {
  rb_header* header;
  char* name;
  int fill_count;
  long event_count;
  long byte_count;
  rb_chunk* first;
  rb_chunk chunk[0];
};

// Meant to be allocated by the user of the event bridge.
struct _rb_state {
    rb_chunk* current; // current chunk used
    unsigned char* write_ptr;
    unsigned char* end_ptr;

    int event_count;   // for current chunk
};

char* shm_init(void);
void shm_startconsumer(char* exe, int);

void shm_finish(void);
char* shm_alloc_segment(Char* name, Int size);
void shm_initialized(void);

/* Allocates a SHM segment. To write events, call shm_init_sending()
 * and use start/end_event() afterwards */
shm_rb* shm_alloc_rb(Char* name, int count, int size);

/* Initialize the sending state to start with first buffer */
void shm_init_sending(rb_state* st, shm_rb* rb);

/* Set current chunk to FULL, and wait for next to allow to fill.
 * This can block (uses spin loop for now) */
rb_chunk* next_chunk(rb_state* st);

/* Set current chunk to FULLEND */
void shm_close(rb_state* st);

/* allow for inlining */


// call before start_event to check for space
// (len must be 2 larger than event size)
static inline
void ensure_space(rb_state* st, int len)
{
    if (st->end_ptr - st->write_ptr < len)
        next_chunk(st);
}

static inline
char* start_event(rb_state* st, char tag, int len)
{
    unsigned char* wp;

    st->event_count++;
    wp = st->write_ptr;

    if(0) VG_(printf)("Starting event %d, len %2d at %p, offset %d.\n",
                      tag, len, wp+2,
                      (int)(wp+2 - st->current->buffer));

    tl_assert2(len <= MAX_EVENTLEN, "length is %d", len);
    wp[0] = len+2;
    wp[1] = tag;

    return wp + 2;
}

// update length of event after start_event()
static inline
void update_length(rb_state* st, int len)
{
    char* wp = st->write_ptr;

    tl_assert2(len <= MAX_EVENTLEN, "length is %d", len);
    *wp = len+2;
}

// finish event after start_event()
static inline
char* end_event(rb_state* st)
{
    unsigned char* wp = st->write_ptr;
    st->write_ptr += *wp;

    return wp;
}

// call if len is known (no need to call end_event afterwards)
static inline
char* write_event(rb_state* st, char tag, int len)
{
    char* b;

    ensure_space(st, len+2);
    b = start_event(st, tag, len);
    st->write_ptr += len+2;

    return b;
}

// same as above, but space already ensured
static inline
char* send_event(rb_state* st, char tag, int len)
{
    char* b;

    b = start_event(st, tag, len);
    st->write_ptr += len+2;

    return b;
}

#endif
