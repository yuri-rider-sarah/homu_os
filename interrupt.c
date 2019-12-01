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

static struct IDTR idtr;

__attribute__((interrupt)) void double_fault_handler(void *frame) {
    print_string(STR("Kernel error: double fault"));
    asm volatile ("hlt");
}

static void set_idt(u32 i, u64 addr) {
    idt[i] = (struct IDT_Entry){(u16)addr, 0x08, 0x8E00, (u16)(addr >> 16), (u32)(addr >> 32), 0};
}

void interrupt_init(void) {
    set_idt(0x08, (u64)&double_fault_handler);
    idtr = (struct IDTR){sizeof(idt) - 1, (u64)&idt};
    asm volatile ("lidt (%0)" : : "r"(&idtr));
    asm volatile ("sti");
}
