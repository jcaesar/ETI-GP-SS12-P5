
#include "pub_tool_basics.h"

void ssim_init(void);
VG_REGPARM(2) void ssim_load(Addr addr, SizeT size);
VG_REGPARM(2) void ssim_store(Addr addr, SizeT size);
void ssim_print_stats(void);
