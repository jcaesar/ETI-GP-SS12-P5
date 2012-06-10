#ifndef _SIMPLESIM_H
#define _SIMPLESIM_H

#include "pub_tool_basics.h"
#include <stdbool.h>

#define MAX_MATRIX_COUNT 256
#define MAX_MATRIX_ACCESS_METHODS 30

struct _element_access_count;
typedef struct _element_access_count
{
	unsigned int hits;
	unsigned int misses;
} element_access_count;

struct _matrix_access_method;
typedef struct _matrix_access_method
{
	/* relative row number */
	short offset_m;
	/* relative column number */
	short offset_n;
	unsigned int misses;
	unsigned int hits;
} matrix_access_method;

struct _matrix_access_data;
typedef struct _matrix_access_data
{
	/* last address accessed by this matrix */
	struct _last_access
	{
		short n;
		short m;
	} last_access;
	/* list of access methods used to retrieve/write data from/to the matrix */
	matrix_access_method access_methods[MAX_MATRIX_ACCESS_METHODS];
	int access_methods_count;
} matrix_access_data;

struct _traced_matrix;
typedef struct _traced_matrix
{
	/* start address of the matrix in memory */
	Addr start;
	/* last address of the matrix in memory */
	Addr end;
	/* name of the matrix */
	char* name;
	/* size of each element of the matrix in bytes */
	unsigned short ele_size;
	/* number of rows */
	unsigned short m;
	/* number of columns */
	unsigned short n;
	/* contains data about the memory and cache access (patterns) */
	matrix_access_data access_data;
	/* number of hits/misses per element for loads */
	element_access_count* load_count;
	/* number of hits/misses per element for stores */
	element_access_count* store_count;
} traced_matrix;

void ssim_init(void);
void ssim_flush_cache(void);
bool ssim_matrix_tracing_start(Addr, unsigned short m, unsigned short n, unsigned short elsize, char*);
bool ssim_matrix_tracing_stop(Addr);
VG_REGPARM(2) void ssim_load(Addr addr, SizeT size);
VG_REGPARM(2) void ssim_store(Addr addr, SizeT size);
void ssim_save_stats(char* fname);

#endif
