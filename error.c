#include "types.h"
#include "error.h"
#include "framebuffer.h"

void kernel_error(String str) {
    print_string(STR("\nKernel error: "));
    print_string(str);
    asm volatile ("cli");
    asm volatile ("hlt");
}
