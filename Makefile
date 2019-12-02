TARGET = homu.bin
AS = nasm
CC = x86_64-elf-gcc
CFLAGS = -ffreestanding -masm=intel -mcmodel=kernel -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -O2 -Wall -Wextra -Wstrict-prototypes
LDFLAGS = -ffreestanding -O2 -nostdlib -lgcc

HEADERS = $(wildcard *.h)
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c)) $(patsubst %.s,%.s.o,$(wildcard *.s))

$(TARGET): $(OBJECTS) linker.ld
	$(CC) $(LDFLAGS) -T linker.ld $(OBJECTS) -o $@
	truncate -s 516096 $(TARGET) # minimum disk size that works with bochs

%.s.o: %.s
	$(AS) $< -f elf64 -o $@

interrupt.o: CFLAGS+=-mgeneral-regs-only

%.o: %.c $(HEADERS)
	$(CC) -c $(CFLAGS) $< -o $@
