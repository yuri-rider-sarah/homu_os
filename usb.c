#include "types.h"
#include "usb.h"
#include "error.h"
#include "interrupt.h"
#include "page_alloc.h"

#include "framebuffer.h"

void usb_host_controller_init(u32 pci_addr);

void usb_init(void) {
    for (u32 addr = 0x80000000; addr < 0x80FFF800; addr += 0x800) {
        u32 id;
        u32 class;
        asm volatile ("out %1, %0" : : "a"(addr), "Nd"((u16)0xCF8));
        asm volatile ("in %0, %1" : "=a"(id) : "Nd"((u16)0xCFC));
        asm volatile ("out %1, %0" : : "a"(addr + 0x08), "Nd"((u16)0xCF8));
        asm volatile ("in %0, %1" : "=a"(class) : "Nd"((u16)0xCFC));
        if ((id >> 16) != 0xFFFF && class >> 8 == 0x0C0320) {
            print_string(STR("Host controller at bus "));
            print_hex((addr >> 16) & 0xFF, 2);
            print_string(STR(", device "));
            print_hex((addr >> 11) & 0x1F, 2);
            print_string(STR(": "));
            usb_host_controller_init(addr);
        }
    }
}

u8 *ehci_regs;
u8 *ehci_op_regs;

__attribute__((interrupt)) void usb_irq_handler(void *frame) {
    print_string(STR("USB IRQ received "));
    print_hex(*(u32 *)(ehci_op_regs + 0x04), 8);
    print_char('\n');
    return;
}

#define EHCI_REGISTERS_REGION 0xFFFFFFFFFFC00000

void usb_host_controller_init(u32 pci_addr) {
    // Setup EHCI regs
    u32 ehci_regs_phys;
    asm volatile ("out %1, %0" : : "a"(pci_addr + 0x10), "Nd"((u16)0xCF8));
    asm volatile ("in %0, %1" : "=a"(ehci_regs_phys) : "Nd"((u16)0xCFC));
    if (ehci_regs_phys == 0)
        kernel_error(STR("No EHCI host controller"));
    ehci_regs = (u8 *)(EHCI_REGISTERS_REGION | (ehci_regs_phys & 0xFFF));
    u64 ehci_regs_pd = page_alloc();
    if (ehci_regs_pd == 0) {
        kernel_error(STR("Out of memory"));
    }
    *PDE_PTR(EHCI_REGISTERS_REGION) = ehci_regs_pd | 0x003;
    *PTE_PTR(EHCI_REGISTERS_REGION) = (ehci_regs_phys & ~0xFFF) | 0x103;
    *(PTE_PTR(EHCI_REGISTERS_REGION) + 1) = ((ehci_regs_phys & 0xFFF) + 0x1000) | 0x103;
    // Get Interrupt Line
    u32 interrupt_line;
    asm volatile ("out %1, %0" : : "a"(pci_addr + 0x3C), "Nd"((u16)0xCF8));
    asm volatile ("in %0, %1" : "=a"(interrupt_line) : "Nd"((u16)0xCFC));
    set_idt(0x20 + (u8)interrupt_line, (u64)usb_irq_handler);
    print_string(STR("\nInterrupt line: "));
    print_hex(interrupt_line, 2);
    print_char('\n');
    // Enable Bus Master and Memory Space
    u32 status_command;
    asm volatile ("out %1, %0" : : "a"(pci_addr + 0x04), "Nd"((u16)0xCF8));
    asm volatile ("in %0, %1" : "=a"(status_command) : "Nd"((u16)0xCFC));
    asm volatile ("out %1, %0" : : "a"(pci_addr + 0x04), "Nd"((u16)0xCF8));
    asm volatile ("out %1, %0" : : "a"(status_command | 0x00000006), "Nd"((u16)0xCFC));
    // Check EHCI Extended Capabilities Pointer
    u32 *hccparams = (u32 *)(ehci_regs + 0x08);
    u8 eecp = *hccparams >> 8;
    if (eecp >= 0x40) {
        // Get ownership of controller
        u32 usblegsup;
        asm volatile ("out %1, %0" : : "a"(pci_addr + eecp), "Nd"((u16)0xCF8));
        asm volatile ("in %0, %1" : "=a"(usblegsup) : "Nd"((u16)0xCFC));
        if ((usblegsup & 0x01010000) != 0x01000000) {
            asm volatile ("out %1, %0" : : "a"(pci_addr + eecp), "Nd"((u16)0xCF8));
            asm volatile ("out %1, %0" : : "a"(usblegsup | 0x01000000), "Nd"((u16)0xCFC));
        }
        while ((usblegsup & 0x01010000) != 0x01000000) {
            asm volatile ("out %1, %0" : : "a"(pci_addr + eecp), "Nd"((u16)0xCF8));
            asm volatile ("in %0, %1" : "=a"(usblegsup) : "Nd"((u16)0xCFC));
        }
    }
    // Stop and reset controller
    ehci_op_regs = ehci_regs + *ehci_regs;
    volatile u32 *usbcmd = (u32 *)ehci_op_regs;
    *usbcmd = (*usbcmd & 0xFFFFFFFE) | 0x00000002;
    while (*usbcmd & 0x00000002)
        ;
    // Setup registers
    *hccparams |= 0x00000001; // 64-bit Addressing Capability
    *(u32 *)(ehci_op_regs + 0x08) = 0x00000017; // USBINTR
    *(u32 *)(ehci_op_regs + 0x10) = 0xFFFFFFFF; // CTRLDSSEGMENT
    // Start controller
    *usbcmd |= 0x00000001;
    // Find devices
    u32 *hcsparams = (u32 *)(ehci_regs + 0x04);
    u32 n_ports = *hcsparams & 0x0000000F;
    print_hex(n_ports, 2);
    print_string(STR(" ports detected\n"));
    volatile u32 *portsc = (u32 *)(ehci_op_regs + 0x40); // indexed from 1 to N_PORTS
    for (u32 i = 1; i <= n_ports; i++) {
        print_string(STR("Port #"));
        print_hex(i, 2);
        print_string(STR(", PORTSC = "));
        print_hex(portsc[i], 8);
        print_char('\n');
    }
}
