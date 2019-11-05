SECTION .boot

bits 16

  cli
  mov ax, 0
  mov ds, ax
  mov es, ax
  mov ss, ax
  mov sp, 0x7C00
  push dx

  clc
  mov ah, 0x41
  mov bx, 0x55AA
  mov dl, 0x80
  int 0x13
  jnc .int13_supported
  mov si, .int13_ext_fail
  call .print_string
  hlt
.int13_supported:
  pop dx
  mov ah, 0x42
  mov si, .int13_dap
  int 0x13
  jnc .loaded_kernel
  mov si, .int13_fail
  call .print_string
  hlt
.loaded_kernel:
  cli
  lgdt [.gdtr]
  mov eax, cr0
  or eax, 1
  mov cr0, eax
  jmp 0x08:.protected_mode_init

align 4
.int13_dap:
  db 0x10
  db 0
  dw 0x10
  dd 0x00007E00
  dq 1

align 8
.gdt:
; null selector
  dw 0
.gdtr:
  dw 0x17
  dd .gdt
; code selector
  dw 0xFFFF
  dw 0x0000
  db 0x00
  db 0x9A
  db 0xCF
  db 0x00
; data selector
  dw 0xFFFF
  dw 0x0000
  db 0x00
  db 0x92
  db 0xCF
  db 0x00

.int13_ext_fail: db `INT 13h extensions not supported\r\n\0`
.int13_fail: db `INT 13h failed\r\n\0`

; Prints string located in SI.
.print_string:
  cld
  mov ah, 0x0E
  lodsb
  test al, al
  jz .end
  int 0x10
  jmp .print_string
.end:
  ret

bits 32

.protected_mode_init:
  mov ax, 0x10
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax
  extern kernel_main
  call kernel_main
  hlt

times 510 - ($-$$) db 0
dw 0xAA55
