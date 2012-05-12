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
#define TR_SIMPLESIM_DEFINE_DATA 4
#define TR_SIMPLESIM_CHANGE_SECTION 5
#define TR_SIMPLESIM_CONFIGURE 6

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

// tag TR_SIMPLESIM_DEFINE_DATA
typedef struct {
  char description[64];
  Addr start;
  int size;
} ev_simplesim_define_data;

// tag TR_SIMPLESIM_CHANGE_SECTION
typedef struct {
  unsigned int id;
  char description[64];
} ev_simplesim_change_section;

// tag TR_SIMPLESIM_CONFIGURE
typedef struct {
  char setting[64];
  int value;
} ev_simplesim_configure;

struct _tr_event {
  /* Event header */
  unsigned char len;
  unsigned char tag;
  union {
    ev_run_tid     run_tid;
    ev_data_read   data_read;
    ev_data_write  data_write;
    ev_simplesim_define_data simplesim_define_data;
		ev_simplesim_change_section simplesim_change_section;
		ev_simplesim_configure simplesim_configure;
  };
};
#pragma pack(pop)

#endif
