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

    print_string(STR("HomuOS\n\nKeyboard test:\n"));
    u8 key;
    while (true) {
        key = kb_buffer_read();
        if (key < sizeof(chars))
            print_char(chars[key]);
    }

    asm volatile ("hlt");
}
