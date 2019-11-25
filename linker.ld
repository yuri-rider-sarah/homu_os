OUTPUT_FORMAT(binary)

SECTIONS {
    . = 0x7C00;
    .boot : AT(0) {
        *(.boot)
    }
    . = 0xFFFF800000008000;
    .text : AT(ADDR(.text) - 0xFFFF800000008000 + 0x400) {
        *(.text)
        *(.rodata.*)
    }
    .data : AT(ADDR(.data) - 0xFFFF800000008000 + 0x400) {
        *(.data)
    }
    .bss : AT(ADDR(.bss) - 0xFFFF800000008000 + 0x400) {
        *(COMMON)
        *(.bss)
    }
    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
    }
}
