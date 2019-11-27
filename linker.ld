OUTPUT_FORMAT(binary)

KERNEL_VMA = 0xFFFFFFFFFFE08000;

SECTIONS {
    . = 0x7C00;
    .boot : AT(0) {
        *(.boot)
    }
    . = KERNEL_VMA;
    .text : AT(ADDR(.text) - KERNEL_VMA + SIZEOF(.boot)) {
        *(.text)
        *(.rodata.*)
    }
    .data : AT(ADDR(.data) - KERNEL_VMA + SIZEOF(.boot)) {
        *(.data)
    }
    .bss : AT(ADDR(.bss) - KERNEL_VMA + SIZEOF(.boot)) {
        *(COMMON)
        *(.bss)
    }
    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
    }
}
