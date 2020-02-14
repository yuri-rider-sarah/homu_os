#include "types.h"
#include "file.h"
#include "drive.h"
#include "mem.h"

/* NOTE: this implementation is currently very broken,
 * but seems to work with small files.
 * It can only be accessed through the FUSE driver.
 */

// All files are referenced by the location of their metadata structure.

static struct NamedFileEntry {
    u8 type;
    union {
        u64 empty_field_size : 56;
        struct {
            u8 reserved[7];
            u64 size;
            u64 block_pointer[16];
            u8 name_len;
            u8 name[FILENAME_MAX];
        } __attribute__((packed));
    };
} __attribute__((packed)) file, file2; // file2 is used by directory operations

#define FILE_ENTRY_SIZE 0x90

static u64 superblock;
static u64 num_blocks;
static u64 block_map_start;

// Returns the address of the root directory
i64 file_system_init(void) {
    RIE(drive_read(0x1F0, 8, &superblock));
    RIE(drive_read(superblock, 8, &num_blocks));
    block_map_start = superblock + 0x1000;
    return superblock + 0xF70;
}

static i64 block_alloc(void) {
    for (u64 i = 0; i < num_blocks; i++) {
        u8 bits;
        RIE(drive_read(block_map_start + i / 8, 1, &bits));
        if (!((bits >> (i % 8)) & 1)) {
            bits |= 1 << (i % 8);
            RIE(drive_write(block_map_start + i / 8, 1, &bits));
            return i << 12;
        }
    }
    return -1;
}

static i64 block_free(u64 block) {
    block >>= 12;
    u8 bits;
    RIE(drive_read(block_map_start + block / 8, 1, &bits));
    bits &= ~(1 << (block % 8));
    RIE(drive_write(block_map_start + block / 8, 1, &bits));
    return 0;
}

static u32 file_indirection(u64 size) {
    return size ? ((64 - __builtin_clzll(size - 1) - 16 + 8) / 9) : 0;
}

static i64 file_locate(u64 addr, u64 offset) {
    RIE(drive_read(addr, FILE_ENTRY_SIZE, &file));
    u32 indirection = file_indirection(file.size);
    u32 part_shift = 12 + 9 * indirection;
    u64 block_pointer = file.block_pointer[offset >> part_shift];
    for (u32 i = 0; i < indirection; i++)
        RIE(drive_read(block_pointer + 8 * ((addr >> part_shift) & 0x1FF), 8, &block_pointer));
    block_pointer += offset & 0xFFF;
    return block_pointer;
}

static i64 file_rw_recurse(i64 (*drive_rw)(u64, u64, void *), u64 start, u64 len, void *buf, u64 table, u32 indirection) {
    u64 end = start + len;
    if (indirection > 0) {
        u64 part_shift = 12 + 9 * indirection;
        u64 part_size = 1 << part_shift;
        for (u64 part_start = start; part_start < end; part_start = (part_start + part_size) & ~(part_size - 1)) {
            u64 part_len = MIN(end - part_start, part_size);
            u64 block_pointer;
            RIE(drive_read(table + 8 * ((part_start >> part_shift) & 0x1FF), 8, &block_pointer));
            RIE(file_rw_recurse(drive_rw, part_start, part_len, buf, block_pointer, indirection - 1));
            buf += part_len;
        }
        return 0;
    } else {
        return drive_rw(table + (start & 0xFFF), len, buf);
    }
}

static i64 file_rw(i64 (*drive_rw)(u64, u64, void *), u64 start, u64 len, void *buf) {
    u64 end = start + len;
    u32 indirection = file_indirection(file.size);
    if (end >= file.size || end < start)
        end = file.size;
    u64 part_shift = 12 + 9 * indirection;
    u64 part_size = 1 << part_shift;
    for (u64 part_start = start; part_start < end; part_start = (part_start + part_size) & ~(part_size - 1)) {
        u64 part_len = MIN(end - part_start, part_size);
        RIE(file_rw_recurse(drive_rw, part_start, part_len, buf,
                    file.block_pointer[part_start >> part_shift], file_indirection(file.size)));
        buf += part_len;
    }
    return len;
}

static i64 free_blocks(u64 start, u64 end, u64 table, u32 indirection) {
    u64 part_shift = 12 + 9 * indirection;
    u64 part_size = 1 << part_shift;
    for (u64 part_start = start; part_start < end; part_start = (part_start + part_size) & ~(part_size - 1)) {
        u64 part_end = MIN(part_start + part_size, end);
        u64 block_pointer;
        RIE(drive_read(table + 8 * (part_start >> part_shift), 8, &block_pointer));
        if (indirection > 0)
            RIE(free_blocks(part_start, part_end, block_pointer, indirection - 1));
        if ((start & (part_size - 1)) == 0)
            RIE(block_free(block_pointer));
    }
    return 0;
}

static i64 alloc_blocks(u64 start, u64 end, u64 table, u32 indirection) {
    u64 part_shift = 12 + 9 * indirection;
    u64 part_size = 1 << part_shift;
    for (u64 part_start = start; part_start < end; part_start = (part_start + part_size) & ~(part_size - 1)) {
        u64 part_end = MIN(part_start + part_size, end);
        i64 block_pointer;
        if ((start & (part_size - 1)) == 0) {
            block_pointer = block_alloc();
            if (block_pointer < 0) {
                file.size = start;
                return -1;
            }
            RIE(drive_write(table + 8 * (part_start >> part_shift), 8, &block_pointer));
        } else if (indirection > 0) {
            RIE(drive_read(table + 8 * (part_start >> part_shift), 8, &block_pointer));
        }
        if (indirection > 0)
            RIE(alloc_blocks(part_start, part_end, block_pointer, indirection - 1));
    }
    return 0;
}

static i64 resize(u64 addr, u64 size, bool blank) {
    if (size < file.size) {
        RIE(free_blocks(size, file.size, addr + offsetof(struct NamedFileEntry, block_pointer), file_indirection(file.size)));
        // Reduce indirection if necessary
        for (u32 i = 0; i < file_indirection(file.size) - file_indirection(size); i++)
            RIE(drive_read(file.block_pointer[0], 16 * 8, &file.block_pointer));
    } else {
        // TODO quickly check if there's no available memory
        // Increase indirection if necessary
        for (u32 i = 0; i < file_indirection(size) - file_indirection(file.size); i++) {
            i64 block = block_alloc();
            if (block < 0) {
                // Reverse indirection
                // TODO fix this
                for (u32 j = 0; j < i; j++)
                    RIE(drive_read(file.block_pointer[0], 16 * 8, &file.block_pointer));
                return -1;
            }
            RIE(drive_write(block, 8 * 16, &file.block_pointer));
            file.block_pointer[0] = block;
        }
        i64 err = alloc_blocks(file.size, size, addr + offsetof(struct NamedFileEntry, block_pointer), file_indirection(file.size));
        RIE(drive_read(addr + offsetof(struct NamedFileEntry, block_pointer), 16 * 8, &file.block_pointer));
        if (err < 0) {
            RIE(resize(addr, file.size, blank)); // Return file to original size
            // Return indirection to previous state
            for (u32 i = 0; i < file_indirection(size) - file_indirection(file.size); i++)
                RIE(drive_read(file.block_pointer[0], 16 * 8, &file.block_pointer));
            return err;
        }
    }
    file.size = size;
    return drive_write(addr, FILE_ENTRY_SIZE, &file);
}

i64 file_read(u64 addr, u64 start, u64 len, void *dest) {
    RIE(drive_read(addr, FILE_ENTRY_SIZE, &file));
    // TODO check file.type without interfering with calls from dir_*
    if (start + len < start || file.size < start + len) // Read until EOF
        len = file.size - start;
    return file_rw(drive_read, start, len, dest);
}

i64 file_write(u64 addr, u64 start, u64 len, void *src) {
    RIE(drive_read(addr, FILE_ENTRY_SIZE, &file));
    // TODO check file.type without interfering with calls from dir_*
    // TODO Avoid overflow in next comparison
    if (file.size < start + len) {
        RIE(resize(addr, start + len, false));
        RIE(drive_read(addr, FILE_ENTRY_SIZE, &file));
    }
    return file_rw(drive_write, start, len, src);
}

i64 file_resize(u64 addr, u64 size) {
    RIE(drive_read(addr, FILE_ENTRY_SIZE, &file));
    if (file.type != FTYPE_FILE)
        return -1;
    return resize(addr, size, true);
}

i64 file_metadata(u64 addr, FileMetadata *md) {
    RIE(drive_read(addr, FILE_ENTRY_SIZE, &file));
    md->type = file.type;
    md->size = file.size;
    return 0;
}

i64 dir_read(u64 addr, u64 *offset, NamedFileMetadata *nmd) {
    i64 r = file_read(addr, *offset, sizeof(file2), &file2);
    RIE(r);
    if (r == 0) {
        *offset = UINT64_MAX;
        return 0;
    }
    if (r < FILE_ENTRY_SIZE)
        return -1;
    if (file2.type == 0xFF) {
        *offset += file2.empty_field_size;
        RIE(file_read(addr, *offset, sizeof(file2), &file2));
    }
    FileMetadata *md = &nmd->md;
    md->type = file2.type;
    md->size = file2.size;
    nmd->name_len = file2.name_len;
    memcpy(&nmd->name, &file2.name, file2.name_len);
    *offset += FILE_ENTRY_SIZE + (file2.name_len + 8) / 8 * 8;
    return 0;
}

static i64 dir_mk(u64 addr, String name, u8 type) {
    if (name.len > FILENAME_MAX)
        return -1;
    u64 size = FILE_ENTRY_SIZE + (name.len + 8) / 8 * 8;
    RIE(drive_read(addr, FILE_ENTRY_SIZE, &file2));
    u64 offset = file2.size;
    file2.type = type;
    file2.size = 0;
    file2.name_len = name.len;
    memcpy(&file2.name, name.chars, name.len);
    RIE(file_write(addr, offset, size, &file2));
    return file_locate(addr, offset);
}

i64 dir_mkfile(u64 addr, String name) {
    return dir_mk(addr, name, FTYPE_FILE);
}

i64 dir_mkdir(u64 addr, String name) {
    return dir_mk(addr, name, FTYPE_DIR);
}

static i64 dir_find(u64 addr, String name) {
    RIE(drive_read(addr, FILE_ENTRY_SIZE, &file2));
    u64 offset = 0;
    while (1) {
        i64 r = file_read(addr, offset, sizeof(file2), &file2);
        RIE(r);
        if (r == 0)
            return -2;
        if (r < FILE_ENTRY_SIZE)
            return -1;
        if (file2.type == 0xFF) {
            offset += file2.empty_field_size;
        } else {
            if (file2.name_len == name.len && memcmp(file2.name, name.chars, name.len) == 0)
                return offset;
            offset += FILE_ENTRY_SIZE + (file2.name_len + 8) / 8 * 8;
        }
    }
}

i64 dir_open(u64 addr, String name) {
    i64 offset = dir_find(addr, name);
    RIE(offset);
    return file_locate(addr, offset);
}
