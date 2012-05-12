/* Shared memory event bridge
 * (C) 2011, Josef Weidendorfer
 *
 * Internal SHM file structure common for both producer and consumer side
 */

#ifndef SHMPRIV_H
#define SHMPRIV_H

/* 7 chars */
#define SHM_MAGIC "EVBRG-1"
#define SHM_NAME  "event_bridge"
#define SHMSIZE (1<<20)

typedef struct {
  char magic[8];
  int  size;
  char producer_64bit;
  char producer_initialized;
  char reserved1;
  char consumer_attached;

  struct {
    int offset;
    int size;
    char name[8];
  } seg[15];
} shm_header;

/* Chunked ring buffers
 *
 * Format:
 * - 64 byte header (rb_header)
 * - chunk state array: for each chunk: 1 byte state, 63 byte padding (1 cacheline)
 * - for each chunk: payload buffer, aligned to 64 bytes
 *   if chunk is full, 4 first bytes of payload give used size
 */

#define RBSTATE_EMPTY   0
#define RBSTATE_FULL    1
#define RBSTATE_FULLEND 2

typedef struct {
  int chunk_count;
  int chunk_size;
  int state0_offset;    /* offset in segment to chunk state array */
  int buffer0_offset;   /* <elem_size> bytes per element */
} rb_header;


#endif /* SHMPRIV_H */
