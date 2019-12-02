#include "types.h"
#include "interrupt.h"
#include "framebuffer.h"

struct IDT_Entry {
    u16 addr0;
    u16 selector;
    u16 type_attr_ist;
    u16 addr1;
    u32 addr2;
    u32 reserved;
};

static struct IDT_Entry idt[0x30];

struct IDTR {
    u16 limit;
    u64 base;
} __attribute__((packed));

struct IDTR idtr;

__attribute__((interrupt)) void double_fault_handler(void *frame) {
    print_string(STR("Kernel error: double fault"));
    asm volatile ("hlt");
}

__attribute__((interrupt)) void keyboard_irq_handler(void *frame) {
    u8 code;
    asm volatile ("in al, 0x60" : "=a"(code));
    print_string(STR("Scan code received: "));
    print_hex(code, 2);
    print_char('\n');
    asm volatile ("out 0x20, al" : : "a"(0x20));
}

static void set_idt(u32 i, u64 addr) {
    idt[i] = (struct IDT_Entry){(u16)addr, 0x08, 0x8E00, (u16)(addr >> 16), (u32)(addr >> 32), 0};
}

extern void int_enable(void);

void interrupt_init(void) {
    set_idt(0x08, (u64)&double_fault_handler);
    set_idt(0x21, (u64)&keyboard_irq_handler);
    idtr = (struct IDTR){sizeof(idt) - 1, (u64)&idt};
    int_enable();
}
