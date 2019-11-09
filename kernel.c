#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

void kernel_main(void) {
    u8 *fb = (u8 *)*(u32 *)0x0728;
    for (u32 i = 0; i < (*(u16 *)0x0732 * *(u16 *)0x0714); i++)
        fb[i] = i;
    while (1)
        ;
}
