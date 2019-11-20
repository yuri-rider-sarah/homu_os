#include "types.h"
#include "misc-fixed.h"

typedef struct String {
    size_t len;
    uint8_t *chars;
} String;

#define STR(s) ((String){sizeof(s) - 1, (uint8_t *)s})

u8 *fb;
u16 pitch;
u16 width;
u16 height;
u16 bytes_per_pixel;
u8 rs;
u8 rp;
u8 gs;
u8 gp;
u8 bs;
u8 bp;

u16 char_width;
u16 char_height;

u32 cursor_x = 0;
u32 cursor_y = 0;

void print_char(uint8_t c) {
    if (c == '\n') {
        cursor_y = (cursor_y + 1) % char_height;
        cursor_x = 0;
    } else if (c >= ' ' && c <= '~') {
        for (u32 y = 0; y < 13; y++)
            for (u32 x = 0; x < 7; x++)
                for (u32 j = 0; j < bytes_per_pixel; j++)
                    fb[(cursor_y * 13 + y) * pitch + (cursor_x * 7 + x) * bytes_per_pixel + j] =
                        ((font_chars[c - ' '][y] << x) & 0x80) ? 0xFF : 0x00;
        cursor_x = (cursor_x + 1) % char_width;
    }
}

void print_string(String str) {
    for (size_t i = 0; i < str.len; i++)
        print_char(str.chars[i]);
}

void print_hex(u64 n, u32 digits) {
    print_char('0');
    print_char('x');
    for (i32 shift = 4 * (digits - 1); shift >= 0; shift -= 4) {
        i32 d = (n >> shift) & 0xF;
        print_char(d < 10 ? d + '0' : d - 10 + 'A');
    }
}

void kernel_main(void) {
    fb = (u8 *)*(u32 *)0x0728;
    pitch = *(u16 *)0x0710;
    width = *(u16 *)0x0712;
    height = *(u16 *)0x0714;
    bytes_per_pixel = (*(u8 *)0x0719 + 7) / 8;
    rs = *(u8 *)0x071F;
    rp = *(u8 *)0x0720;
    gs = *(u8 *)0x0721;
    gp = *(u8 *)0x0722;
    bs = *(u8 *)0x0723;
    bp = *(u8 *)0x0724;
    char_width = width / 7;
    char_height = height / 13;
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
    print_string(STR("HomuOS\n"));
    print_string(STR("Hello, world!\n"));
    print_hex(0x01234567, 8);
    print_char('\n');
    print_hex(0x89ABCDEF, 16);
    print_char('\n');
    while (1)
        ;
}
