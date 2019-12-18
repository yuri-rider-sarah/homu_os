OUTPUT_FORMAT(binary)

KERNEL_VMA = 0xFFFFFFFFFFE08000;

SECTIONS {
    . = 0x7C00;
    .boot : AT(0) {
        *(.boot)
    }
    . = KERNEL_VMA;
    .text ALIGN(8) : AT(ADDR(.text) - KERNEL_VMA + SIZEOF(.boot)) {
        *(.text)
        *(.rodata*)
    }
    .data ALIGN(8) : AT(ADDR(.data) - KERNEL_VMA + SIZEOF(.boot)) {
        *(.data)
    }
    .bss ALIGN(8) : AT(ADDR(.bss) - KERNEL_VMA + SIZEOF(.boot)) {
        *(COMMON)
        *(.bss)
    }
    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
    }
}
