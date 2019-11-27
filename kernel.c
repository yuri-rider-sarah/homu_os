#include "types.h"
#include "misc-fixed.h"

#define PTE_PTR(x)   ((u64 *)(((u64)x >>  9 & 0x0000007FFFFFFFF8) | 0xFFFF800000000000))
#define PDE_PTR(x)   ((u64 *)(((u64)x >> 18 & 0x000000003FFFFFF8) | 0xFFFF804000000000))
#define PDPTE_PTR(x) ((u64 *)(((u64)x >> 27 & 0x00000000001FFFF8) | 0xFFFF804020000000))
#define PML4E_PTR(x) ((u64 *)(((u64)x >> 36 & 0x0000000000000FF8) | 0xFFFF804020100000))

#define LOW_MEM_PTR(x) (x | 0xFFFFFFFFFFE00000)

typedef struct String {
    size_t len;
    uint8_t *chars;
} String;

#define STR(s) ((String){sizeof(s) - 1, (uint8_t *)s})

typedef struct MemoryRegion {
    u64 base;
    u64 len;
    u32 type;
    u32 attrs;
} MemoryRegion;

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

#define PAGE_STACK_BOTTOM (u64 *)0xFFFFFFFF80000000

static u64 *page_stack_top = PAGE_STACK_BOTTOM;
static u64 *page_stack_capacity = PAGE_STACK_BOTTOM;

void free_page(u64 page);

void page_alloc_init() {
    u64 *page_stack_pdpte = PDPTE_PTR(PAGE_STACK_BOTTOM);
    u16 memory_ranges_count = *(u16 *)LOW_MEM_PTR(0x08FE) / 24;
    MemoryRegion *memory_ranges = (MemoryRegion *)LOW_MEM_PTR(0x0900);
    for (u16 i = 0; i < memory_ranges_count; i++) {
        if (memory_ranges[i].type != 1 || memory_ranges[i].base <= 0xFFFFF) // Skip invalid ranges and low memory
            continue;
        u64 base_page = (memory_ranges[i].base + 0xFFF) >> 12;
        u64 end_page = (memory_ranges[i].base + memory_ranges[i].len) >> 12;
        if (base_page >= end_page) // Skip ranges not containing a full page
            continue;
        if (*page_stack_pdpte == 0) {
            *page_stack_pdpte = base_page << 12 | 0x003;
            u64 *page_stack_pd = PDE_PTR(PAGE_STACK_BOTTOM);
            for (u32 i = 0; i < 0x200; i++)
                page_stack_pd[i] = 0;
            base_page++;
            if (base_page >= end_page)
                continue;
        }
        for (u64 page = base_page; page < end_page; page++)
            free_page(page << 12);
    }
    // TODO handle no usable memory
}

void free_page(u64 page) {
    if (page_stack_top >= page_stack_capacity) {
        u64 *page_stack_cap_pde = PDE_PTR(page_stack_capacity);
        u64 *page_stack_cap_pt = PTE_PTR(page_stack_capacity);
        if (*page_stack_cap_pde == 0) {
            *page_stack_cap_pde = page | 0x103;
            for (u32 i = 0; i < 0x200; i++)
                page_stack_cap_pt[i] = 0;
        } else {
            *page_stack_cap_pt = page | 0x103;
            page_stack_capacity += 0x200;
        }
    } else {
        *page_stack_top++ = page;
    }
    // TODO handle too much memory
}

u64 page_alloc() {
    if (page_stack_top == PAGE_STACK_BOTTOM)
        return 0;
    return *--page_stack_top;
}

void kernel_main(void) {
    *PDE_PTR(0) = 0;
    *PDPTE_PTR(0) = 0;
    *PML4E_PTR(0) = 0;
    asm volatile ("invlpg 0" : : : "memory");

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

    fb = (u8 *)(0xFFFFFFFF40000000 | (fb_ptr & 0x1FFFFF));
    *PDPTE_PTR(fb) = 0x7C003;
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

    page_alloc_init();

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

    print_string(STR("Address of kernel_main: "));
    print_hex(&kernel_main, 16);
    print_char('\n');
    u16 memory_ranges_count = *(u16 *)LOW_MEM_PTR(0x08FE) / 24;
    MemoryRegion *memory_ranges = (MemoryRegion *)LOW_MEM_PTR(0x0900);
    print_string(STR("Detected memory:\n"));
    for (u16 i = 0; i < memory_ranges_count; i++) {
        print_hex(memory_ranges[i].base, 16);
        print_char(' ');
        print_hex(memory_ranges[i].len, 16);
        print_char(' ');
        print_hex(memory_ranges[i].type, 8);
        print_char('\n');
    }
    print_char('\n');
    print_string(STR("Page allocation test:\n"));
    u64 page1 = page_alloc();
    u64 page2 = page_alloc();
    print_hex(page1, 16);
    print_char('\n');
    print_hex(page2, 16);
    print_char('\n');
    free_page(page1);
    free_page(page2);
    page1 = page_alloc();
    page2 = page_alloc();
    print_hex(page1, 16);
    print_char('\n');
    print_hex(page2, 16);
    print_char('\n');

    while (1)
        ;
}
