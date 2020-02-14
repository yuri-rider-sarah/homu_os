#include "types.h"
#include "drive.h"
#include "error.h"
#include "mem.h"

#define SECTOR_LENGTH 512
#define SECTORS_IN_BUFFER 512
#define DRIVE_IO_BUFFER ((void *)0xFFFFFFFFFFE40000)

extern i64 int13(u8 function, u64 start, u16 len);

i64 drive_read(u64 start, u64 len, void *dest) {
    u64 sector_start = start / SECTOR_LENGTH; // First sector to read
    u64 sector_len = (start + len + SECTOR_LENGTH - 1) / SECTOR_LENGTH - sector_start; // Number of sectors to read
    u64 offset = start - sector_start * SECTOR_LENGTH; // Offset of `start` from beginning of sector
    if (sector_len > SECTORS_IN_BUFFER)
        kernel_error(STR("`read_drive` called with `len` too large"));
    RIE(int13(0x42, sector_start, sector_len));
    memcpy(dest, DRIVE_IO_BUFFER + offset, len);
    return 0;
}

i64 drive_write(u64 start, u64 len, void *src) {
    u64 sector_start = start / SECTOR_LENGTH;
    u64 sector_end = (start + len + SECTOR_LENGTH - 1) / SECTOR_LENGTH; // Sector after last sector written
    u64 sector_len = sector_end - sector_start;
    u64 offset = start - sector_start * SECTOR_LENGTH;
    u64 end_offset = sector_end * SECTOR_LENGTH - (start + len); // Difference between end of last sector and end of write
    if (sector_len > SECTORS_IN_BUFFER)
        kernel_error(STR("`write_drive` called with `len` too large"));
    // Read first and last sector if necessary
    if (end_offset != 0 && !(sector_len == 1 && offset != 0)) {
        RIE(int13(0x42, sector_end - 1, 1));
        memcpy(DRIVE_IO_BUFFER + offset + len, DRIVE_IO_BUFFER + offset + len - (sector_len - 1) * SECTOR_LENGTH, end_offset);
    }
    if (offset != 0)
        RIE(int13(0x42, sector_start, 1));
    memcpy(DRIVE_IO_BUFFER + offset, src, len);
    return int13(0x43, sector_start, sector_len);
}
