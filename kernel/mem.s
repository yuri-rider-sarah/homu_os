global memcpy
global memcmp

;void memcpy(void *dest, void *src, u64 bytes);
memcpy:
  mov rcx, rdx
  rep movsb
  ret

memcmp:
  mov rcx, rdx
  mov rdx, -1
  mov rax, 0
  repe cmpsb
  setb al
  cmova rax, rdx
  ret
