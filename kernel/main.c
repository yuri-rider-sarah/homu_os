#include "types.h"
#include "framebuffer.h"
#include "error.h"
#include "file.h"
#include "interrupt.h"
#include "keyboard.h"
#include "keycodes.h"
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

u8 shift_chars[105] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '~', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0,
    0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '|',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '\n',
    0, 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    0, 0, 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '/', '*', '-', '+', '\n', '.', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
};

void kernel_main(void) {
    framebuffer_init();
    page_alloc_init();
    ps2_init();
    i64 fs_init_ret = file_system_init();
    if (fs_init_ret < 0)
        kernel_error(STR("Failed to initialize file system"));
    u64 root_dir = (u64)fs_init_ret;
    interrupt_init();

    print_string(STR("HomuOS\n\n"));

    print_string(STR("Keyboard test:\n"));
    bool shift = false;
    while (true) {
        u8 key = kb_buffer_read();
        if (key == KEY_LEFT_SHIFT || key == KEY_RIGHT_SHIFT)
            shift = true;
        else if (key == (KEY_LEFT_SHIFT | KEY_RELEASED) || key == (KEY_RIGHT_SHIFT | KEY_RELEASED))
            shift = false;
        else if (key < sizeof(chars))
            print_char((shift ? shift_chars : chars)[key]);
    }

    asm volatile ("hlt");
}
