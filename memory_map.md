The following memory ranges are used by the kernel:
- PML4E `0x100`: `0xFFFF800000000000` - `0xFFFF807FFFFFFFFF` - recursive mapping of page tables
- PML4E `0x1FF`:
  - PDPTE `0x1FD`: `0xFFFFFFFF40000000` - `0xFFFFFFFF7FFFFFFF` - framebuffer
  - PDPTE `0x1FE`: `0xFFFFFFFF80000000` - `0xFFFFFFFFBFFFFFFF` - page frame stack
  - PDPTE `0x1FF`:
    - PDE `0x001`:
      - PTE `0x000`: `0xFFFFFFFFC0400000` - `0xFFFFFFFFC0400FFF` - stack
    - PDE `0x1FF`: `0xFFFFFFFFFFE00000` - `0xFFFFFFFFFFFFFFFF` - first 2 MiB of physical memory (includes code)
