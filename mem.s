global memcpy

;void memcpy(void *dest, void *src, u64 bytes);
memcpy:
  mov rcx, rdx
  rep movsb
  ret
