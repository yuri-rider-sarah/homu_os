#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

void kernel_main(void) {
    u8 *fb = (u8 *)*(u32 *)0x0728;
    u16 pitch = *(u16 *)0x0710;
    u16 width = *(u16 *)0x0712;
    u16 height = *(u16 *)0x0714;
    u16 bytes_per_pixel = (*(u8 *)0x0719 + 7) / 8;
    u8 rs = *(u8 *)0x071F;
    u8 rp = *(u8 *)0x0720;
    u8 gs = *(u8 *)0x0721;
    u8 gp = *(u8 *)0x0722;
    u8 bs = *(u8 *)0x0723;
    u8 bp = *(u8 *)0x0724;
    for (u32 y = 0; y < height; y++) {
        for (u32 x = 0; x < width; x++) {
            u8 r = x;
            u8 g = y;
            u8 b = 0;
            u32 pixel = (r >> (8 - rs)) << rp | (g >> (8 - gs)) << gp | (b >> (8 - bs)) << bp;
            for (u32 i = 0; i < bytes_per_pixel; i++) {
                fb[y * pitch + x * bytes_per_pixel + i] = (pixel >> 8 * i);
            }
        }
    }
    while (1)
        ;
}
