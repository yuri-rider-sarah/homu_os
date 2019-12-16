The following memory ranges are used by the kernel:
- PML4E `0x000`:
  - PDPTE `0x000`:
    - PDE `0x000`: `0x0000000000000000` - `0x00000000001FFFFF` - first 2 MiB of physical memory (identity mapping for use with real mode)
- PML4E `0x100`: `0xFFFF800000000000` - `0xFFFF807FFFFFFFFF` - recursive mapping of page tables
- PML4E `0x1FF`:
  - PDPTE `0x1FD`: `0xFFFFFFFF40000000` - `0xFFFFFFFF7FFFFFFF` - framebuffer
  - PDPTE `0x1FE`: `0xFFFFFFFF80000000` - `0xFFFFFFFFBFFFFFFF` - page frame stack
  - PDPTE `0x1FF`:
    - PDE `0x001`:
      - PTE `0x000`: `0xFFFFFFFFC0400000` - `0xFFFFFFFFC0400FFF` - stack
    - PDE `0x1FF`: `0xFFFFFFFFFFE00000` - `0xFFFFFFFFFFFFFFFF` - first 2 MiB of physical memory (includes code)

Low memory map:
- `0x00000` - `0x004FF` - BIOS data
- `0x00500` - `0x06000` - data obtained from BIOS
- `0x06000` - `0x07000` - kernel stack
- `0x07C00` - `0x07FFF` - bootloader
- `0x08000` - `0x3FFFF` - kernel code/data
- `0x40000` - `0x7FFFF` - disk read/write buffer
- `0x80000` - `0xFFFFF` - BIOS data
