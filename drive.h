#include "types.h"

u32 read_drive(u64 start, u64 len, void *dest);
u32 write_drive(u64 start, u64 len, void *src);
