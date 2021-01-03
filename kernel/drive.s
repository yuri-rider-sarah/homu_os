extern gdtr
extern gdtr32
extern idtr

global int13

%define drive_index 0x08FC

section .data

align 8
gdt16:
  ; null selector
  dq 0
  ; code selector
  dw 0xFFFF
  dw 0x0000
  db 0x00
  db 0x9A
  db 0x0F
  db 0x00
  ; data selector
  dw 0xFFFF
  dw 0x0000
  db 0x00
  db 0x92
  db 0x0F
  db 0x00

gdtr16:
  dw 0x17
  dq gdt16 - 0xFFFFFFFFFFE00000

idtr16:
  dw 0x03FF
  dq 0x00000000

align 4
int13_dap:
  db 0x10
  db 0
.len: dw 0
  dd 0x40000000
.start: dq 0

section .text

; u32 int13(u8 function, u64 start, u16 len)
int13:
  push rbx
  push rbp
  ; Setup DAP
  mov [int13_dap.len], dx
  mov [int13_dap.start], rsi
  ; Enter real mode
  lgdt [gdtr16]
  lidt [idtr16]
  add rsp, 0x7000 - 0xFFFFFFFFC0400000
  push 0x08
  push .protected_mode - 0xFFFFFFFFFFE00000
  retfq ; load 16-bit code selector
.protected_mode:
bits 16
  mov ax, 0x10
  mov ss, ax
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov eax, cr0
  and eax, 0x7FFFFFFE ; Leave protected mode
  mov cr0, eax
  jmp 0:(.real_mode - 0xFFFFFFFFFFE00000)
.real_mode:
  mov ax, 0
  mov ss, ax
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  ; Read data from drive
  shl di, 8
  mov ax, di
  mov dl, [drive_index]
  mov si, int13_dap - 0xFFFFFFFFFFE00000
  int 0x13
  ; Store return value in al
  test ah, ah
  setz al
  mov dx, 0
  cmovc ax, dx
  ; Return to long mode
  cli
  lgdt [gdtr32]
  mov edx, cr0
  or edx, 0x80000001
  mov cr0, edx
  jmp 0x08:(.long_mode_low - 0xFFFFFFFFFFE00000)
.long_mode_low:
bits 64
  jmp .long_mode
.long_mode:
  lgdt [gdtr + 0xFFFFFFFFFFE00000]
  lidt [idtr]
  add rsp, 0xFFFFFFFFC0400000 - 0x7000
  mov dx, 0x10
  mov ss, dx
  mov ds, dx
  mov es, dx
  mov fs, dx
  mov gs, dx
  pop rbp
  pop rbx
  and rax, 1
  ret
