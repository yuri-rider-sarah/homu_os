wait_for_read:
  in al, 0x64
  test al, 0x01
  jz wait_for_read
  ret

wait_for_write:
  in al, 0x64
  test al, 0x02
  jnz wait_for_write
  ret

global ps2_init

ps2_init:
  ; set Controller Configuration Byte
  call wait_for_write
  mov al, 0x20
  out 0x64, al
  call wait_for_read
  in al, 0x60
  and al, 0xBF
  or al, 0x03
  push rax
  call wait_for_write
  mov al, 0x60
  out 0x64, al
  call wait_for_write
  pop rax
  out 0x60, al
  ret
