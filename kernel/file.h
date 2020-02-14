#include "types.h"

#define FILENAME_MAX 255

enum {
    FTYPE_FILE,
    FTYPE_DIR,
} FileType;

typedef struct FileMetadata {
    u8 type;
    u64 size;
} FileMetadata;

typedef struct NamedFileMetadata {
    FileMetadata md;
    u8 name_len;
    u8 name[FILENAME_MAX];
} NamedFileMetadata;

i64 file_system_init(void);
i64 file_read(u64 addr, u64 start, u64 len, void *dest);
i64 file_write(u64 addr, u64 start, u64 len, void *src);
i64 file_resize(u64 addr, u64 size);
i64 file_metadata(u64 addr, FileMetadata *md);
i64 dir_open(u64 addr, String name);
i64 dir_read(u64 addr, u64 *offset, NamedFileMetadata *nmd);
i64 dir_mkfile(u64 addr, String name);
i64 dir_mkdir(u64 addr, String name);
