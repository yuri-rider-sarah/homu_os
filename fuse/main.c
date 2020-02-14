#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FUSE_USE_VERSION 37
#include <fuse3/fuse.h>

#include "file.h"

void *s_malloc(size_t n) {
    void *p = malloc(n);
    if (p == NULL)
        exit(1);
    return p;
}

static u64 root_dir;

static i64 open_(String str0) {
    String str = str0;
    if (str.len > 0 && str.chars[0] == '/')
        str = (String){str.len - 1, str.chars + 1};
    u64 addr = root_dir;
    while (str.len > 0) {
        char *first_slash = strchr((char *)str.chars, '/');
        if (first_slash == NULL || first_slash - (char *)str.chars > str.len)
            first_slash = str.chars + str.len;
        String name = (String){first_slash - (char *)str.chars, str.chars};
        printf("dir_open(0x%lX, %s/%d)\n", addr, name.chars, name.len);
        i64 r = dir_open(addr, name);
        RIE(r);
        addr = (u64)r;
        if (first_slash == str.chars + str.len)
            break;
        str = (String){str.len - name.len - 1, str.chars + name.len + 1};
    }
    return addr;
}

static int homu_read(const char *path, char *dest, size_t len, off_t start, struct fuse_file_info *fi) {
    printf("file_read(0x%lX (%s), 0x%lX, 0x%lX, <dest>)\n", fi->fh, path, start, len);
    return file_read(fi->fh, start, len, dest);
}

static int homu_write(const char *path, const char *src, size_t len, off_t start, struct fuse_file_info *fi) {
    printf("file_write(0x%lX (%s), 0x%lX, 0x%lX, <src>)\n", fi->fh, path, start, len);
    return file_write(fi->fh, start, len, (void *)src);
}

static int homu_truncate(const char *path, off_t size, struct fuse_file_info *fi) {
    printf("file_resize(0x%lX (%s), 0x%lX)\n", fi->fh, path, size);
    return file_resize(fi->fh, size);
}

static int homu_getattr(const char *path, struct stat *st, struct fuse_file_info *fi) {
    printf("homu_getattr(%s)\n", path);
    memset(st, 0, sizeof(struct stat));
    i64 r0 = open_((String){strlen(path), path});
    RIE(r0);
    u64 addr = (u64)r0;
    printf("file_metadata(0x%lX (%s), <md>)\n", addr, path);
    FileMetadata *md = s_malloc(sizeof(FileMetadata));
    i64 r = file_metadata(addr, md);
    if (r < 0) {
        free(md);
        return r;
    }
    switch (md->type) {
    case FTYPE_FILE:
        st->st_mode = S_IFREG | 0644;
        break;
    case FTYPE_DIR:
        st->st_mode = S_IFDIR | 0755;
        break;
    default:
        return -1;
    }
    st->st_size = md->size;
    st->st_nlink = 2;
    st->st_uid = getuid();
    st->st_gid = getgid();
    free(md);
    return 0;
}

static int homu_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset_,
        struct fuse_file_info *fi, enum fuse_readdir_flags flags) {
    printf("homu_readdir(%s)\n", path);
    u64 offset = 0;
    NamedFileMetadata *nmd = s_malloc(sizeof(NamedFileMetadata));
    while (1) {
        printf("dir_read(0x%lX (%s), 0x%lX, <nmd>)\n", fi->fh, path, offset);
        RIE(dir_read(fi->fh, &offset, nmd));
        if (offset == UINT64_MAX)
            break;
        char *name = s_malloc(nmd->name_len + 1);
        memcpy(name, nmd->name, nmd->name_len);
        name[nmd->name_len] = '\0';
        printf("Found %s\n", name);
        if (filler(buf, name, NULL, 0, 0) == 1) {
            free(nmd);
            free(name);
            return -1;
        }
        free(name);
    }
    free(nmd);
    return 0;
}

static int homu_open(const char *path, struct fuse_file_info *fi) {
    i64 r = open_((String){strlen(path), path});
    RIE(r);
    fi->fh = (u64)r;
    return 0;
}

static i64 create_(const char *path, i64 (*dir_mk)(u64, String)) {
    char *last_slash = strrchr(path, '/');
    if (last_slash == NULL)
        return -1;
    i64 dir_r = open_((String){last_slash - path, path});
    RIE(dir_r);
    return dir_mk((u64)dir_r, (String){strlen(last_slash + 1), last_slash + 1});
}

static int homu_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    printf("homu_create(%s)\n", path);
    i64 r = create_(path, dir_mkfile);
    RIE(r);
    fi->fh = r;
    return 0;
}

static int homu_mkdir(const char *path, mode_t mode) {
    printf("homu_mkdir(%s)\n", path);
    return create_(path, dir_mkdir);
}

static struct fuse_operations homufs_ops = {
    .read = homu_read,
    .write = homu_write,
    .truncate = homu_truncate,
    .getattr = homu_getattr,
    .open = homu_open,
    .opendir = homu_open,
    .readdir = homu_readdir,
    .create = homu_create,
    .mkdir = homu_mkdir,
};

static struct fuse_opt option_spec[] = {
    { "--image=%s", 0, 1 },
    FUSE_OPT_END,
};

extern FILE *image;

int main(int argc, char **argv) {
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    char *image_path = NULL;
    if (fuse_opt_parse(&args, &image_path, option_spec, NULL) != 0) {
        printf("Failed to parse arguments\n");
        return 1;
    }
    if (image_path == NULL) {
        printf("No image= argument\n");
        return 1;
    }
    image = fopen(image_path, "r+b");
    if (image == NULL) {
        printf("Failed to read image file\n");
        return errno;
    }
    i64 fs_init_ret = file_system_init();
    if (fs_init_ret < 0) {
        printf("Failed to initialize file system\n");
        return 1;
    }
    root_dir = (u64)fs_init_ret;
    return fuse_main(args.argc, args.argv, &homufs_ops, NULL);
}
