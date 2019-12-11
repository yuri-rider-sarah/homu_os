#include "types.h"
#include "usb.h"
#include "error.h"
#include "page_alloc.h"

u32 find_usb_host_controller(void) {
    for (u32 addr = 0x80000000; addr < 0x80FFF800; addr += 0x800) {
        u32 id;
        u32 class;
        u32 bar0;
        asm volatile ("out %1, %0" : : "a"(addr), "Nd"((u16)0xCF8));
        asm volatile ("in %0, %1" : "=a"(id) : "Nd"((u16)0xCFC));
        asm volatile ("out %1, %0" : : "a"(addr + 0x08), "Nd"((u16)0xCF8));
        asm volatile ("in %0, %1" : "=a"(class) : "Nd"((u16)0xCFC));
        asm volatile ("out %1, %0" : : "a"(addr + 0x10), "Nd"((u16)0xCF8));
        asm volatile ("in %0, %1" : "=a"(bar0) : "Nd"((u16)0xCFC));
        if ((id >> 16) != 0xFFFF && class >> 8 == 0x0C0320) {
            return bar0;
        }
    }
    return 0;
}

#define EHCI_REGISTERS_REGION 0xFFFFFFFFFFC00000

u8 *usb_init(void) {
    u32 ehci_registers_phys = find_usb_host_controller();
    if (ehci_registers_phys == 0)
        kernel_error(STR("No EHCI host controller"));
    u8 *ehci_registers = (u8 *)(EHCI_REGISTERS_REGION | (ehci_registers_phys & 0xFFF));
    u64 ehci_registers_pd = page_alloc();
    if (ehci_registers_pd == 0) {
        kernel_error(STR("Out of memory"));
    }
    *PDE_PTR(EHCI_REGISTERS_REGION) = ehci_registers_pd | 0x003;
    *PTE_PTR(EHCI_REGISTERS_REGION) = (ehci_registers_phys & ~0xFFF) | 0x103;
    *(PTE_PTR(EHCI_REGISTERS_REGION) + 1) = ((ehci_registers_phys & 0xFFF) + 0x1000) | 0x103;
    return ehci_registers;
}
