#include "types.h"
#include "error.h"
#include "interrupt.h"
#include "keyboard.h"

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
    kernel_error(STR("Double fault"));
}

void set_idt(u8 i, u64 addr) {
    idt[i] = (struct IDT_Entry){(u16)addr, 0x08, 0x8E00, (u16)(addr >> 16), (u32)(addr >> 32), 0};
}

extern void int_enable(u16 irq_mask);

void interrupt_init(void) {
    set_idt(0x08, (u64)&double_fault_handler);
    set_idt(0x21, (u64)&keyboard_irq_handler);
    u16 irq_mask = 0x0000;
    for (u32 i = 0; i < 0x10; i++) {
        if (idt[0x20 + i].selector == 0x00)
            irq_mask |= (1 << i);
    }
    irq_mask &= 0xFFFB;
    idtr = (struct IDTR){sizeof(idt) - 1, (u64)&idt};
    int_enable(irq_mask);
}
