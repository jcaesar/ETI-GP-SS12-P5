/* Shared memory event bridge (consumer side)
 * Allows multiple, chunked ring buffers
 *
 * (C) 2011, Josef Weidendorfer
 */

#ifndef SHM_CONSUMER_H
#define SHM_CONSUMER_H

typedef struct _shm_buf shm_buf;
typedef struct _rb_chunk rb_chunk;
typedef struct _shm_rb shm_rb;

shm_buf* attach(int pid);
shm_rb* open_rb(shm_buf*, char* name);
rb_chunk* open_first(shm_rb*);
rb_chunk* finish_chunk(rb_chunk* c, rb_chunk** cPtr);
unsigned char* next_event(rb_chunk** cPtr);

shm_buf* shm_init(int argc, char* argv[]); // parses <pid> arg and attaches

void shm_printf(const char *format, ...);

#endif
