TARGET = homufs_fuse
CC = gcc
CFLAGS = -O2 -Wall -Wstrict-prototypes -funsigned-char `pkg-config fuse3 --cflags --libs`
LDFLAGS = -O2 `pkg-config fuse3 --cflags --libs`

HEADERS = $(wildcard *.h)
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c)) $(patsubst %.s,%.s.o,$(wildcard *.s))

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

%.o: %.c $(HEADERS)
	$(CC) -c $(CFLAGS) $< -o $@
