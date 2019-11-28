#include "framebuffer.h"
#include "misc-fixed.h"

static u8 *fb;
static u16 pitch;
static u16 width;
static u16 height;
static u16 bytes_per_pixel;
static u8 rs;
static u8 rp;
static u8 gs;
static u8 gp;
static u8 bs;
static u8 bp;

static u16 char_width;
static u16 char_height;

static u32 cursor_x = 0;
static u32 cursor_y = 0;

void framebuffer_init() {
    u32 fb_ptr = *(u32 *)LOW_MEM_PTR(0x0728);
    pitch = *(u16 *)LOW_MEM_PTR(0x0710);
    width = *(u16 *)LOW_MEM_PTR(0x0712);
    height = *(u16 *)LOW_MEM_PTR(0x0714);
    bytes_per_pixel = (*(u8 *)LOW_MEM_PTR(0x0719) + 7) / 8;
    rs = *(u8 *)LOW_MEM_PTR(0x071F);
    rp = *(u8 *)LOW_MEM_PTR(0x0720);
    gs = *(u8 *)LOW_MEM_PTR(0x0721);
    gp = *(u8 *)LOW_MEM_PTR(0x0722);
    bs = *(u8 *)LOW_MEM_PTR(0x0723);
    bp = *(u8 *)LOW_MEM_PTR(0x0724);
    char_width = width / 7;
    char_height = height / 13;

    fb = (u8 *)(0xFFFFFFFF40000000 | (fb_ptr & 0x1FFFFF));
    *PDPTE_PTR(fb) = 0x7B003;
    u64 *fb_pd = PDE_PTR(fb);
    u64 fb_first_page = fb_ptr >> 21;
    u64 fb_end_page = (fb_ptr + height * pitch + 0x1FFFFF) >> 21;
    u64 fb_pages = fb_end_page - fb_first_page;
    if (fb_pages > 0x200)
        fb_pages = 0x200;
    for (u32 i = 0; i < fb_end_page - fb_first_page; i++)
        fb_pd[i] = (fb_first_page + i) << 21 | 0x000183;
    for (u32 i = fb_end_page - fb_first_page; i < 0x200; i++)
        fb_pd[i] = 0;
}

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
