#include "types.h"
#include "framebuffer.h"
#include "page_alloc.h"

typedef struct MemoryRegion {
    u64 base;
    u64 len;
    u32 type;
    u32 attrs;
} MemoryRegion;

void kernel_main(void) {
    *PDE_PTR(0) = 0;
    *PDPTE_PTR(0) = 0;
    *PML4E_PTR(0) = 0;
    asm volatile ("invlpg 0" : : : "memory");

    framebuffer_init();
    page_alloc_init();

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
