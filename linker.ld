OUTPUT_FORMAT(binary)

MEMORY {
    boot : ORIGIN = 0x7C00, LENGTH = 512
    kernel : ORIGIN = 0x7E00, LENGTH = 8K
}

SECTIONS {
    .boot : {
        *(.boot)
    } >boot
    .text : {
        *(.text)
    } >kernel
    .rodata : {
        *(.rodata)
    } >kernel
    .data : {
        *(.data)
    } >kernel
    .bss : {
        *(COMMON)
        *(.bss)
    } >kernel
    /DISCARD/ : {
    }
}
