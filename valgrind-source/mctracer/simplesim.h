#ifndef _SIMPLESIM_H
#define _SIMPLESIM_H

#include "pub_tool_basics.h"
#include <stdbool.h>
#include <stdint.h>

#define MAX_MATRIX_COUNT 256
#define MAX_MATRIX_ACCESS_METHODS 30

#define BA_MAX_HIT_VAL 254
#define BA_NO_ACCESS_VAL 255

#define MATRIX_LOAD 'L'
#define MATRIX_STORE 'S'

typedef struct _matrix_coordinates // helper structure for any sort of coordinates
{
	short m; // row
	short n; // column
} matrix_coordinates;
static inline bool coordinates_equal(matrix_coordinates a, matrix_coordinates b) { return a.m == b.m && a.n == b.n; }

struct _element_access_count;
typedef struct _element_access_count // hit miss counter for matrix (absolute) stats
{
	unsigned int hits;
	unsigned int misses;
} element_access_count;

struct _matrix_access_method;
typedef struct _matrix_access_method // one access method (relative matrix access, see docs) and its hit/miss statistics
{
	matrix_coordinates offset; // relative
	unsigned int misses;
	unsigned int hits;
} matrix_access_method;

struct _matrix_access_data;
typedef struct _matrix_access_data // substructure to each matrix (twice, for loads and stores) Manages the access methods
{
	/* last address accessed by this matrix */
	matrix_coordinates last_access;
	/* list of access methods used to retrieve/write data from/to the matrix */
	matrix_access_method access_methods[MAX_MATRIX_ACCESS_METHODS];
	int access_methods_count;
} matrix_access_data;

#define MATRIX_ACCESS_ANALYSIS_BUFFER_LENGTH (1<<16)
typedef struct _access_event // for buffering a lot of accesses in a matrix for pattern/sequence
{
	bool is_hit;
	matrix_coordinates offset; // relative coordinates of that access method
} access_event;

struct _access_pattern;
typedef struct _pattern_sequence // a pattern sequence. Managed by the pattern it belongs to
{
	unsigned int length; // how many times did we observe the pattern subsequently?
	struct _access_pattern * next_pattern; // which pattern did we observe next? 0 for no pattern
	matrix_access_method next_access; // which access did we observe next?
	unsigned int occurences; // how many times did we observe this sequence?
} pattern_sequence;

#define MAX_MAX_PATTERN_LENGTH (MATRIX_ACCESS_ANALYSIS_BUFFER_LENGTH/8)
typedef struct _access_pattern
{
	unsigned int length; // size of steps. 0 is used to indicate an inactive pattern in continous arrays
	matrix_access_method * steps; // the accesses belonging to that pattern. The algorithms don't really care which is first, oonly order matters
	unsigned int occurences; // accesses = ouccurences * length
	unsigned int accesses_before_lifetime; // for deciding which pattern to purge
	pattern_sequence * sequences; // Sequences consisting of that pattern
	unsigned int sequence_count; // length of sequences (used)
	unsigned int sequence_allocated; // length of sequences (allocated)
} access_pattern;

//for writing sequences into file
#define MAX_SEQUENCES_PER_MATRIX 255

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
	access_event * access_buffer; // buffer for making pattern/sequence possible
	unsigned int access_event_count; // how full access_buffer is
	matrix_coordinates last_access; // unlike load/store_access_data's last_access: does not distinguish between stores and loads
	/* access patterns */
	access_pattern * access_patterns; // all the access patterns of a matrix. length is always clo_ssim_max_patterns_per_matrix. Can contain "holes". Unused patterns are marked by 0 length
	access_pattern * current_pattern; // for helping find_sequences smooth over the buffer end
	unsigned int current_sequence_length; // for find_sequences
} traced_matrix;

extern traced_matrix traced_matrices [MAX_MATRIX_COUNT];
extern traced_matrix* traced_matrices_index[MAX_MATRIX_COUNT];
extern int traced_matrices_count;
extern int stopped_matrices_count;

void ssim_init(Int sets_, Int setsize_);
void ssim_flush_cache(void);
bool ssim_matrix_tracing_start(Addr, unsigned short m, unsigned short n, unsigned short elsize, char*);
bool ssim_matrix_tracing_stop(Addr);
VG_REGPARM(2) void ssim_load(Addr addr, SizeT size);
VG_REGPARM(2) void ssim_store(Addr addr, SizeT size);
void ssim_save_stats(char* fname);

void update_matrix_stats(Addr addr, SizeT size, char type);
void process_pattern_buffer(traced_matrix * matr);
void update_matrix_pattern_stats(traced_matrix *, unsigned short m, unsigned short n, bool is_hit);
traced_matrix* find_matrix(Addr access);
int cache_ref(Addr a, int size);

extern Int clo_ssim_max_pattern_length;
extern Int clo_ssim_max_patterns_per_matrix;

#endif
