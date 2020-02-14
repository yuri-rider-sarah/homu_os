#include <errno.h>
#include <stdio.h>

#include "types.h"
#include "drive.h"

FILE *image;

i64 drive_read(u64 start, u64 len, void *dest) {
    printf("drive_read(0x%lX, 0x%lX, <dest>)\n", start, len);
    if (start == 0) {
        printf("Attempt to read at address 0\n");
        exit(-1);
    }
    if (fseek(image, start, SEEK_SET) != 0) {
        printf("fseek() failed with %d\n", errno);
        return -1;
    }
    if (fread(dest, 1, len, image) < len) {
        printf("fread() failed with %d\n", errno);
        return -1;
    }
    return len;
}

i64 drive_write(u64 start, u64 len, void *src) {
    printf("drive_write(0x%lX, 0x%lX, <src>)\n", start, len);
    if (start == 0) {
        printf("Attempt to write at address 0\n");
        exit(-1);
    }
    if (fseek(image, start, SEEK_SET) != 0) {
        printf("fseek() failed with %d\n", errno);
        return -1;
    }
    if (fwrite(src, 1, len, image) < len) {
        printf("fwrite() failed with %d\n", errno);
        return -1;
    }
    return len;
}
