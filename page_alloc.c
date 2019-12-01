#include "types.h"
#include "page_alloc.h"

typedef struct MemoryRegion {
    u64 base;
    u64 len;
    u32 type;
    u32 attrs;
} MemoryRegion;

#define PAGE_STACK_BOTTOM (u64 *)0xFFFFFFFF80000000

static u64 *page_stack_top = PAGE_STACK_BOTTOM;
static u64 *page_stack_capacity = PAGE_STACK_BOTTOM;

void page_alloc_init(void) {
    u64 *page_stack_pdpte = PDPTE_PTR(PAGE_STACK_BOTTOM);
    u16 memory_ranges_count = *(u16 *)LOW_MEM_PTR(0x08FE) / 24;
    MemoryRegion *memory_ranges = (MemoryRegion *)LOW_MEM_PTR(0x0900);
    for (u16 i = 0; i < memory_ranges_count; i++) {
        if (memory_ranges[i].type != 1 || (memory_ranges[i].attrs & 3) != 1 || memory_ranges[i].base <= 0xFFFFF) // Skip invalid ranges and low memory
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
        for (u64 page = base_page; page < end_page; page++) {
            // Add page to page stack
            if (page_stack_top >= page_stack_capacity) {
                u64 *page_stack_cap_pde = PDE_PTR(page_stack_capacity);
                u64 *page_stack_cap_pt = PTE_PTR(page_stack_capacity);
                if (*page_stack_cap_pde == 0) {
                    *page_stack_cap_pde = page << 12 | 0x103;
                    for (u32 i = 0; i < 0x200; i++)
                        page_stack_cap_pt[i] = 0;
                } else {
                    *page_stack_cap_pt = page << 12 | 0x103;
                    page_stack_capacity += 0x200;
                }
            } else {
                *page_stack_top++ = page << 12;
            }
            // TODO handle too much memory
        }
    }
    // TODO handle no usable memory
}

u64 page_alloc(void) {
    if (page_stack_top == PAGE_STACK_BOTTOM)
        return 0;
    return *--page_stack_top;
}

void free_page(u64 page) {
    *page_stack_top++ = page;
}
