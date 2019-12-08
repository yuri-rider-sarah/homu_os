#include "types.h"
#include "framebuffer.h"
#include "interrupt.h"
#include "keyboard.h"
#include "page_alloc.h"

extern void ps2_init(void);

u8 chars[105] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '`', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,
    0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\\',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '\n',
    0, 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    0, 0, 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '/', '*', '-', '+', '\n', '.', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
};

void kernel_main(void) {
    *PDE_PTR(0) = 0;
    *PDPTE_PTR(0) = 0;
    *PML4E_PTR(0) = 0;
    asm volatile ("invlpg [0]" : : : "memory");

    framebuffer_init();
    interrupt_init();
    page_alloc_init();
    ps2_init();

    for (u32 addr = 0x80000000; addr < 0x80FFF800; addr += 0x800) {
        u32 data;
        u32 class;
        asm volatile ("out %1, %0" : : "a"(addr), "Nd"((u16)0xCF8));
        asm volatile ("ind %0, %1" : "=a"(data) : "Nd"((u16)0xCFC));
        asm volatile ("out %1, %0" : : "a"(addr + 0x08), "Nd"((u16)0xCF8));
        asm volatile ("ind %0, %1" : "=a"(class) : "Nd"((u16)0xCFC));
        u16 vendor_id = data >> 16;
        if (vendor_id != 0xFFFF) {
            print_string(STR("Valid PCI device at bus "));
            print_hex((addr >> 16) & 0xFF, 2);
            print_string(STR(", number "));
            print_hex((addr >> 11) & 0x1F, 2);
            print_string(STR(", class etc. "));
            print_hex(class, 8);
            print_char('\n');
        }
    }

    print_string(STR("HomuOS\n\nKeyboard test:\n"));
    u8 key;
    while (true) {
        key = kb_buffer_read();
        if (key < sizeof(chars))
            print_char(chars[key]);
    }

    asm volatile ("hlt");
}
