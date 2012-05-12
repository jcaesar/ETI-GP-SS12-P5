/*
 * McTracer: memory event tracer via event bridge.
 * Definition of used event types.
 *
 * For ETI @ TUM, (C) 2011 Josef Weidendorfer
 */

#ifndef TR_SHMEVENTS_H
#define TR_SHMEVENTS_H

#define TR_RUN_TID           1
#define TR_DATA_READ         2
#define TR_DATA_WRITE        3

typedef struct _tr_event tr_event;

#pragma pack(push)
#pragma pack(1)

// tag TR_RUN_TID
typedef struct {
  int tid;
} ev_run_tid;

// tag TR_DATA_READ
typedef struct {
  Addr addr;
  char len;
} ev_data_read;

// tag TR_DATA_WRITE
typedef struct {
  Addr addr;
  char len;
} ev_data_write;

struct _tr_event {
  /* Event header */
  unsigned char len;
  unsigned char tag;
  union {
    ev_run_tid     run_tid;
    ev_data_read   data_read;
    ev_data_write  data_write;
  };
};
#pragma pack(pop)

#endif
