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

void kernel_main(void) {
    framebuffer_init();
    page_alloc_init();
    ps2_init();
    interrupt_init();

    print_string(STR("HomuOS\n\n"));

    u16 x;
    read_drive(510, 2, &x);
    print_string(x == 0xAA55 ? STR("Read test passed\n") : STR("Read test not passed\n"));
    x = 0x0123;
    write_drive(510, 2, &x);
    x = 0xFFFF;
    read_drive(510, 2, &x);
    print_string(x == 0x0123 ? STR("Write test passed\n") : STR("Write test not passed\n"));
    x = 0xAA55;
    write_drive(510, 2, &x);

    print_string(STR("Keyboard test:\n"));
    u8 key;
    while (true) {
        key = kb_buffer_read();
        if (key < sizeof(chars))
            print_char(chars[key]);
    }

    asm volatile ("hlt");
}
