The following memory ranges are used by the kernel:
- `0xFFFF800000000000` - `0xFFFF807FFFFFFFFF` - recursive mapping of page tables
- `0xFFFFFFFF40000000` - `0xFFFFFFFF7FFFFFFF` - framebuffer
- `0xFFFFFFFF80000000` - `0xFFFFFFFFBFFFFFFF` - page frame stack
- `0xFFFFFFFFC0400000` - `0xFFFFFFFFC0400FFF` - stack
- `0xFFFFFFFFFFE00000` - `0xFFFFFFFFFFFFFFFF` - first 2 MiB of physical memory (includes code)
