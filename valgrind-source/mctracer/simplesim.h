
#include "pub_tool_basics.h"
#include <stdbool.h>

#define MAX_MATRIX_COUNT 256

void ssim_init(void);
void ssim_flush_cache(void);
bool ssim_matrix_allocated(Addr, int x, int y, int elsize, char*);
bool ssim_matrix_freed(Addr);
VG_REGPARM(2) void ssim_load(Addr addr, SizeT size);
VG_REGPARM(2) void ssim_store(Addr addr, SizeT size);
void ssim_print_stats(void);
