void memcpy(void *dest, const void *src, u64 bytes);
i64 memcmp(const void *p1, const void *p2, u64 bytes);

#define memcpy __builtin_memcpy
#define memcmp __builtin_memcmp
