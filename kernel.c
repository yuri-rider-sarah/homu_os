#include <stdint.h>

void kernel_main(void) {
    *(uint8_t *)0xB8000 = 'H';
    *(uint8_t *)0xB8001 = 0x05;
    *(uint8_t *)0xB8002 = 'o';
    *(uint8_t *)0xB8003 = 0x05;
    *(uint8_t *)0xB8004 = 'm';
    *(uint8_t *)0xB8005 = 0x05;
    *(uint8_t *)0xB8006 = 'u';
    *(uint8_t *)0xB8007 = 0x05;
    *(uint8_t *)0xB8008 = 'O';
    *(uint8_t *)0xB8009 = 0x05;
    *(uint8_t *)0xB800A = 'S';
    *(uint8_t *)0xB800B = 0x05;
    while (1)
        ;
}
