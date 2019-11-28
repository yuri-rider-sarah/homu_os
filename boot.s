section .boot

%define .controller_info 0x0500
%define .video_modes_ptr 0x050E
%define .mode_info 0x0700
%define .mode_res 0x0712 ; horizontal before vertical
%define .mode_bpp 0x0719
%define .memory_ranges_count 0x08FE ; in bytes
%define .memory_ranges 0x0900

; Error codes:
; 0 - Long mode not supported
; 1 - Failed to enable A20 gate
; 2 - Failed to detect memory
; 3 - INT 13h extensions not supported
; 4 - Failed to load kernel
; 5 - Failed to get controller info
; 6 - Failed to find video mode
; 7 - Failed to get mode info
; 8 - Failed to set video mode

bits 16

  mov ax, 0
  mov ds, ax
  mov es, ax
  mov ss, ax
  mov sp, 0x7000
  push dx
  clc

  ; Test long mode
  mov eax, 0x80000000
  cpuid
  cmp eax, 0x80000001
  jb .no_long_mode
  mov eax, 0x80000001
  cpuid
  test edx, 0x20000000
  jnz .long_mode_available
.no_long_mode:
  mov dl, '0'
  jmp .error
.long_mode_available:

  ; Enable A20 line
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
  je .a20_enabled
  mov ax, 0x2401
  int 0x15
  jc .int15_failed
  test ah, ah
  jz .a20_enabled
.int15_failed:
  mov dl, '1'
  jmp .error
.a20_enabled:

  ; Detect memory ranges
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
  jmp .got_memory_ranges
.detect_memory_failed:
  mov dl, '2'
  jmp .error
.got_memory_ranges:

  ; Load kernel
  mov ah, 0x41
  mov bx, 0x55AA
  mov dl, 0x80
  int 0x13
  jnc .int13_supported
  mov dl, '3'
  jmp .error
.int13_supported:
  pop dx
  mov ah, 0x42
  mov si, .int13_dap
  int 0x13
  jnc .get_video_modes
  mov dl, '4'
  jmp .error

.get_video_modes:
  mov [.controller_info], dword "VBE2"
  mov ax, 0x4F00
  mov di, .controller_info
  int 0x10
  cmp ax, 0x004F
  je .got_controller_info
  mov dl, '5'
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
  mov dl, '6'
  cmp bp, 0xFFFF
  je .error
  mov cx, bp
  mov ax, 0x4F01
  mov di, .mode_info
  int 0x10
  mov dl, '7'
  cmp ax, 0x004F
  jne .error
.got_mode_info:
  mov bx, bp
  or bx, 0x4000
  mov ax, 0x4F02
  int 0x10
  cmp ax, 0x004F
  je .boot
  mov dl, '8'
  jmp .error

align 4
.int13_dap:
  db 0x10
  db 0
  dw 0x11
  dd 0x00007E00
  dq 1

align 8
.gdt:
  dw 0
.gdtr:
  dw 0x17
  dd .gdt
  dq 0x00209A0000000000
  dq 0x0000920000000000

.idtr:
  dw 0
  dd 0

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

times 510 - ($-$$) db 0
dw 0xAA55

.boot:
  cli
  lgdt [.gdtr]
  lidt [.idtr]
  ; clear PDP and PML4
  cld
  mov ax, 0x7000
  mov es, ax
  mov di, 0xE000
  mov edi, 0x7C000
  mov ecx, 0x1000
  xor eax, eax
  rep stosd
  ; setup basic paging - the lowest 2 MiB initially mapped to both 0x0000000000000000 and 0xFFFFFFFFFFE00000
  ; and stack bottom mapped to 0xFFFFFFFFFFC00000
  mov di, 0xCFF8 ; last entry of stack PT
  mov [es:di], dword 0x00006103
  mov di, 0xDFF0 ; second to last entry of PD
  mov [es:di], dword 0x0007C003
  mov di, 0xDFF8 ; last entry of PD
  mov [es:di], dword 0x00000183
  mov di, 0xEFF8 ; last entry of PDPT
  mov [es:di], dword 0x0007D003
  mov di, 0xFFF8 ; last entry of PML4
  mov [es:di], dword 0x0007E003
  mov di, 0xD000 ; first entry of PD
  mov [es:di], dword 0x00000183
  mov di, 0xE000 ; first entry of PDPT
  mov [es:di], dword 0x0007D003
  mov di, 0xF000 ; first entry of PML4
  mov [es:di], dword 0x0007E003
  mov di, 0xF800 ; first entry of lower half of PML4 - set to the PML4 to allow modifying page tables
  mov [es:di], dword 0x0007F003
  ; enable PAE and PGE bits
  mov eax, cr4
  or eax, 0x000000A0
  mov cr4, eax
  ; set cr3 to address of PML4
  mov eax, 0x7F000
  mov cr3, eax
  ; enable long mode
  mov ecx, 0xC0000080
  rdmsr
  or eax, 0x00000100
  wrmsr
  mov eax, cr0
  or eax, 0x80000001
  mov cr0, eax
  jmp 0x08:.long_mode_init

bits 64

extern kernel_main

.long_mode_init:
  mov ax, 0x10
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax
  add rsp, 0xFFFFFFFFFFE00000 - 0x7000
  call kernel_main
.halt:
  hlt
  jmp .halt

times 1024 - ($-$$) db 0
