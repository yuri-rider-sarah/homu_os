TARGET = homu.bin
AS = nasm
CC = i686-elf-gcc
CFLAGS = -ffreestanding -O2 -Wall -Wextra
LDFLAGS = -ffreestanding -O2 -nostdlib -lgcc

HEADERS = $(wildcard *.h)
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c)) $(patsubst %.s,%.o,$(wildcard *.s))

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -T linker.ld $(OBJECTS) -o $@
	truncate -s 8704 $(TARGET)

boot.o: boot.s
	$(AS) $< -f elf32 -o $@

%.o: %.c $(HEADERS)
	$(CC) -c $(CFLAGS) $< -o $@
