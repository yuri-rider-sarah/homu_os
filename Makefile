TARGET = homu.bin
AS = nasm
CC = x86_64-elf-gcc
CFLAGS = -ffreestanding -mcmodel=kernel -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -O2 -Wall -Wextra
LDFLAGS = -ffreestanding -O2 -nostdlib -lgcc

HEADERS = $(wildcard *.h)
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c)) $(patsubst %.s,%.o,$(wildcard *.s))

$(TARGET): $(OBJECTS) linker.ld
	$(CC) $(LDFLAGS) -T linker.ld $(OBJECTS) -o $@
	truncate -s 516096 $(TARGET) # minimum disk size that works with bochs

boot.o: boot.s
	$(AS) $< -f elf64 -o $@

%.o: %.c $(HEADERS)
	$(CC) -c $(CFLAGS) $< -o $@
