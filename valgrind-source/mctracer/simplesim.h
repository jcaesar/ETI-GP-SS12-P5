#ifndef _SIMPLESIM_H
#define _SIMPLESIM_H

#include "pub_tool_basics.h"
#include <stdbool.h>

#define MAX_MATRIX_COUNT 256
#define MAX_MATRIX_ACCESS_METHODS 30

#define BA_MAX_HIT_VAL 254
#define BA_NO_ACCESS_VAL 255

#define MATRIX_LOAD 'L'
#define MATRIX_STORE 'S'

typedef struct _matrix_coordinates {
	short m;
	short n;
} matrix_coordinates;

#define byte unsigned char
#define ushort unsigned short

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
	matrix_coordinates last_access;
	/* list of access methods used to retrieve/write data from/to the matrix */
	matrix_access_method access_methods[MAX_MATRIX_ACCESS_METHODS];
	int access_methods_count;
} matrix_access_data;

#define MATRIX_ACCESS_ANALYSIS_BUFFER_LENGTH (1<<12) // current implementation requires an array of bools with that length to fit onto the stack
typedef struct _access_event {
	bool is_hit;
	matrix_coordinates offset;
} access_event;

struct _access_pattern;
typedef struct _pattern_sequence {
	unsigned int length; // how many times did we observe the pattern subsequently?
	struct _access_pattern * next_pattern; // which pattern did we observe next? 0 for no pattern
	matrix_access_method next_access; // which access did we observe next?
	unsigned int occurences; // how many times did we observe this sequence?
} pattern_sequence;

#define MAX_PATTERNS_PER_MATRIX 5 // TODO: negociate proper value. Probably has to be a nice deal larger than what we display to the user
#define MAX_PATTERN_LENGTH 16
typedef struct _access_pattern {
	unsigned int length;
	matrix_access_method * steps;
	unsigned int occurences; // accesses = ouccurences * length
	unsigned int accesses_before_lifetime; // for deciding which pattern to purge
	pattern_sequence * sequences;
	unsigned int sequence_count;
	unsigned int sequence_allocated;
} access_pattern;

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
	matrix_access_data load_access_data;
    matrix_access_data store_access_data;
	/* number of hits/misses per element for loads */
	element_access_count* load_count;
	/* number of hits/misses per element for stores */
	element_access_count* store_count;
    /* accumulated number of hits/misses for loads over the whole matrix */
    element_access_count loads;
    /* accumulated number of hits/misses for stores over the whole matrix*/
    element_access_count stores;
	/* buffer for finding access patterns */
	access_event * access_buffer; // relative
	unsigned int access_event_count;
	/* access patterns */
	access_pattern access_patterns[MAX_PATTERNS_PER_MATRIX];
	access_pattern * current_pattern;
	unsigned int current_sequence_length;
} traced_matrix;

extern traced_matrix traced_matrices [MAX_MATRIX_COUNT];
extern traced_matrix* traced_matrices_index[MAX_MATRIX_COUNT];
extern int traced_matrices_count;
extern int stopped_matrices_count;

void ssim_init(void);
void ssim_flush_cache(void);
bool ssim_matrix_tracing_start(Addr, unsigned short m, unsigned short n, unsigned short elsize, char*);
bool ssim_matrix_tracing_stop(Addr);
VG_REGPARM(2) void ssim_load(Addr addr, SizeT size);
VG_REGPARM(2) void ssim_store(Addr addr, SizeT size);
void ssim_save_stats(char* fname);

void update_matrix_stats(Addr addr, SizeT size, char type);
traced_matrix* find_matrix(Addr access);
int cache_ref(Addr a, int size);

#endif
