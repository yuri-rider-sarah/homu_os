#pragma once

#include <stddef.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef struct String {
    size_t len;
    uint8_t *chars;
} String;

#define STR(s) ((String){sizeof(s) - 1, (uint8_t *)s})

#define PTE_PTR(x)   ((u64 *)(((u64)x >>  9 & 0x0000007FFFFFFFF8) | 0xFFFF800000000000))
#define PDE_PTR(x)   ((u64 *)(((u64)x >> 18 & 0x000000003FFFFFF8) | 0xFFFF804000000000))
#define PDPTE_PTR(x) ((u64 *)(((u64)x >> 27 & 0x00000000001FFFF8) | 0xFFFF804020000000))
#define PML4E_PTR(x) ((u64 *)(((u64)x >> 36 & 0x0000000000000FF8) | 0xFFFF804020100000))

#define LOW_MEM_PTR(x) (x | 0xFFFFFFFFFFE00000)
