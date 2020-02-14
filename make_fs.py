#!/usr/bin/env python3

import sys

def u64_bytes(n):
    r = []
    for _ in range(8):
        r.append(n & 0xFF)
        n >>= 8
    return bytearray(r)

binary_unit = {'K': 1024, 'M': 1024**2, 'G': 1024**3, 'T': 1024**4}

[filename, size] = sys.argv[1:]
try:
    size = int(size)
except ValueError:
    size = int(size[:-1]) * binary_unit[size[-1]]
size = int(size)
num_blocks = size // 4096
block_map_blocks = -(-num_blocks // 32768) # round up to integer number of pages
reserved_blocks = 5 + block_map_blocks
with open(filename, 'wb') as f:
    f.write(u64_bytes(size))
    f.write(bytearray(3944))
    f.write(bytearray([0x01]))
    f.write(bytearray(143))
    f.write(bytearray(reserved_blocks // 8 * [0xFF]))
    f.write(bytearray([0xFF >> (8 - reserved_blocks % 8)]))
    f.write(bytearray(block_map_blocks * 4096 - reserved_blocks // 8 - 1))
