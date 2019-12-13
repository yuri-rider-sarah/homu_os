extern idtr
global int_enable

int_enable:
  ; di contains IRQ mask
  mov di, 0xF7F9
  lidt [idtr]
  mov al, 0x11
  out 0x20, al
  out 0xA0, al
  mov al, 0x20
  out 0x21, al
  mov al, 0x28
  out 0xA1, al
  mov al, 0x04
  out 0x21, al
  mov al, 0x02
  out 0xA1, al
  mov al, 0x01
  out 0x21, al
  out 0xA1, al
  mov ax, di
  out 0x21, al
  shr ax, 8
  out 0xA1, al
  ret
