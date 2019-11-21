section .boot

%define .controller_info 0x0500
%define .video_modes_ptr 0x050E
%define .mode_info 0x0700
%define .mode_res 0x0712 ; horizontal before vertical
%define .mode_bpp 0x0719
%define .memory_ranges_count 0x08FE ; in bytes
%define .memory_ranges 0x0900

; Error codes:
; 0 - Failed to enable A20 gate
; 1 - Failed to detect memory
; 2 - INT 13h extensions not supported
; 3 - Failed to load kernel
; 4 - Failed to get controller info
; 5 - Failed to get mode info
; 6 - Failed to set video mode

bits 16

  cli
  mov ax, 0
  mov ds, ax
  mov es, ax
  mov ss, ax
  mov sp, 0x7C00
  push dx
  clc

.enable_a20:
  mov ax, 0x2403
  int 0x15
  jc .int15_failed
  test ah, ah
  jnz .int15_failed
  mov ax, 0x2402
  int 0x15
  jc .int15_failed
  test ah, ah
  jnz .int15_failed
  cmp al, 0x01
  je .detect_memory
  mov ax, 0x2401
  int 0x15
  jc .int15_failed
  test ah, ah
  jz .detect_memory
.int15_failed:
  mov dl, '0'
  jmp .error

.detect_memory:
  mov si, .detect_memory_failed
  mov edx, 0x534D4150
  mov ebx, 0
  mov di, .memory_ranges
.detect_memory_loop:
  mov eax, 0xE820
  mov ecx, 24
  int 0x15
  jnc .int15_no_carry
  jmp si
.int15_no_carry:
  cmp eax, 0x534D4150
  jne .detect_memory_failed
  mov si, .got_memory
  add di, 24
  test ebx, ebx
  jnz .detect_memory_loop
.got_memory:
  sub di, .memory_ranges
  mov [.memory_ranges_count], di
  jmp .load_kernel
.detect_memory_failed:
  mov dl, '1'
  jmp .error

.load_kernel:
  mov ah, 0x41
  mov bx, 0x55AA
  mov dl, 0x80
  int 0x13
  jnc .int13_supported
  mov dl, '2'
  jmp .error
.int13_supported:
  pop dx
  mov ah, 0x42
  mov si, .int13_dap
  int 0x13
  jnc .get_video_modes
  mov dl, '3'
  jmp .error

.get_video_modes:
  mov [.controller_info], dword "VBE2"
  mov ax, 0x4F00
  mov di, .controller_info
  int 0x10
  cmp ax, 0x004F
  je .got_controller_info
  mov dl, '4'
  jmp .error
.got_controller_info:
  cld
  mov si, [.video_modes_ptr]
  mov bp, 0xFFFF ; best supported mode
  xor edx, edx ; resolution of best mode - vertical less significant than horizontal
  xor bl, bl ; bpp of best mode
.video_mode_loop:
  lodsw
  cmp ax, 0xFFFF
  je .got_video_mode
  mov cx, ax
  mov ax, 0x4F01
  mov di, .mode_info
  int 0x10
  cmp ax, 0x004F
  jne .video_mode_loop
  test [.mode_info], word 0x0099
  jz .video_mode_loop
  mov eax, [.mode_res]
  ror eax, 16 ; swap horizontal and vertical resolution
  cmp eax, edx
  jb .video_mode_loop
  cmp [.mode_bpp], bl
  jb .video_mode_loop
  mov bp, cx
  mov edx, eax
  mov bl, [.mode_bpp]
  jmp .video_mode_loop
.got_video_mode:
  cmp bp, 0xFFFF
  je .get_mode_info_failed
  mov cx, bp
  mov ax, 0x4F01
  mov di, .mode_info
  int 0x10
  cmp ax, 0x004F
  je .got_mode_info
.get_mode_info_failed:
  mov dl, '5'
  jmp .error
.got_mode_info:
  mov bx, bp
  or bx, 0x4000
  mov ax, 0x4F02
  int 0x10
  cmp ax, 0x004F
  je .boot
  mov dl, '6'
  jmp .error

.boot:
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

.error_msg: db `Error \0`

; Halts with error code in dl displayed
.error:
  cld
  mov si, .error_msg
  mov ah, 0x0E
.print_loop:
  lodsb
  test al, al
  jz .end_print
  int 0x10
  jmp .print_loop
.end_print:
  mov al, dl
  int 0x10
  hlt
  jmp .end_print

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
