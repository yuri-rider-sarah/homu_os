section .boot

%define .controller_info 0x0500
%define .video_modes_ptr 0x050E
%define .mode_info 0x0700
%define .mode_res 0x0712 ; horizontal before vertical

bits 16

  cli
  mov ax, 0
  mov ds, ax
  mov es, ax
  mov ss, ax
  mov sp, 0x7C00
  push dx

  clc
  mov ax, 0x4F00
  mov di, .controller_info
  int 0x10
  cmp ax, 0x004F
  je .got_controller_info
  mov si, .int10_fail
  call .print_string
  hlt
.got_controller_info:
  cld
  mov si, [.video_modes_ptr]
  mov bx, 0xFFFF ; best supported mode
  xor edx, edx ; resolution of best mode - vertical less significant than horizontal
.video_mode_loop:
  lodsw
; or ax, 0x4000
  cmp ax, 0xFFFF
  je .got_video_mode
  mov cx, ax
  mov ax, 0x4F01
  mov di, .mode_info
  int 0x10
  cmp ax, 0x004F
  jne .video_mode_loop
  test [.mode_info], word 0x0010
  jz .video_mode_loop
  mov eax, [.mode_res]
  ror eax, 16 ; swap horizontal and vertical resolution
  cmp eax, edx
  jna .video_mode_loop
  mov bx, cx
  mov edx, eax
  jmp .video_mode_loop
.got_video_mode:
  mov cx, bx
  mov ax, 0x4F01
  mov di, .mode_info
  int 0x10
  cmp ax, 0x004F
  je .got_mode_info
  mov [.int10_fail_digit], byte '1'
  mov si, .int10_fail
  call .print_string
  hlt
.got_mode_info:
  or bx, 0x4000
  mov ax, 0x4F02
  int 0x10
  cmp ax, 0x004F
  je .enable_a20
  mov [.int10_fail_digit], byte '2'
  mov si, .int10_fail
  call .print_string
  hlt

.enable_a20:
  mov ax, 0x2403
  int 0x15
  jc .a20_failed
  test ah, ah
  jnz .a20_failed
  mov ax, 0x2402
  int 0x15
  jc .a20_failed
  test ah, ah
  jnz .a20_failed
  cmp al, 0x01
  je .load_kernel
  mov ax, 0x2401
  int 0x15
  jc .a20_failed
  test ah, ah
  jz .load_kernel
.a20_failed:
  mov si, .int15_fail
  call .print_string
  hlt

.load_kernel:
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

.int10_fail: db `INT 10h AX=0x4F0`
.int10_fail_digit: db `0 failed\r\n\0`
.int13_ext_fail: db `INT 13h extensions not supported\r\n\0`
.int13_fail: db `INT 13h failed\r\n\0`
.int15_fail: db `INT 15h failed\r\n\0`

; Prints string located in SI.
.print_string:
  cld
  mov ah, 0x0E
.print_string_loop:
  lodsb
  test al, al
  jz .end
  int 0x10
  jmp .print_string_loop
.end:
  ret

bits 32

extern kernel_main

.protected_mode_init:
  mov ax, 0x10
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax
  call kernel_main
  hlt

times 510 - ($-$$) db 0
dw 0xAA55
