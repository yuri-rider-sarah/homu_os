#include "types.h"
#include "framebuffer.h"
#include "error.h"
#include "interrupt.h"
#include "keyboard.h"
#include "page_alloc.h"

#include "drive.h"

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

u8 q[4096];

void kernel_main(void) {
    framebuffer_init();
    page_alloc_init();
    ps2_init();
    interrupt_init();

    print_string(STR("HomuOS\n\n"));

    read_drive(0, 1, q);
    u8 *p = (u8 *)0xFFFFFFFFFFE07C00;
    u32 passed = 0;
    for (u32 i = 0; i < 0x1000; i++)
        if (p[i] == q[i])
            passed++;
    print_string(STR("Read test: "));
    print_hex(passed, 4);
    print_string(STR("/0x1000 bytes agree\n"));

    for (u32 i = 0; i < 0x1000; i++)
        q[i] = (u8)i;
    write_drive(0, 1, q);
    for (u32 i = 0; i < 0x1000; i++)
        q[i] = 0xFF;
    read_drive(0, 1, q);
    passed = 0;
    for (u32 i = 0; i < 0x1000; i++)
        if (q[i] == (u8)i)
            passed++;
    print_string(STR("Write/read test: "));
    print_hex(passed, 4);
    print_string(STR("/0x1000 bytes agree\n"));

    print_string(STR("Keyboard test:\n"));
    u8 key;
    while (true) {
        key = kb_buffer_read();
        if (key < sizeof(chars))
            print_char(chars[key]);
    }

    asm volatile ("hlt");
}
