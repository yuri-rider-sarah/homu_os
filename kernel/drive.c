#include "types.h"
#include "drive.h"
#include "error.h"
#include "mem.h"

extern u32 int13(u8 function, u64 start, u16 len);

// `len` must not exceed 64.
u32 read_drive(u64 start, u64 len, void *dest) {
    if (len > 64)
        kernel_error(STR("`len` parameter to `read_drive` too large"));
    u32 success = int13(0x42, start * 8, len * 8); // convert 4K blocks to 512B sectors
    if (!success)
        return 0;
    memcpy(dest, (void *)0xFFFFFFFFFFE40000, len * 4096);
    return 1;
}

u32 write_drive(u64 start, u64 len, void *src) {
    if (len > 64)
        kernel_error(STR("`len` parameter to `write_drive` too large"));
    memcpy((void *)0xFFFFFFFFFFE40000, src, len * 4096);
    return int13(0x43, start * 8, len * 8);
}
