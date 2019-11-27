#include "types.h"

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

void framebuffer_init();
void print_char(uint8_t c);
void print_string(String str);
void print_hex(u64 n, u32 digits);
