#ifndef sfs_h
#define sfs_h

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_NAME_LEN		    483
#define MAX_SECTORS_PER_FILE	1024
//#define DIR_ENTRY_SIZE          512
//#define FCB_SIZE_IN_BLOCKS      5120
//#define FCB_SIZE_IN_BYTES       (FCB_SIZE_IN_BLOCKS * SECTOR_SIZE * 1024)
//#define FCB_MAX_ENTRIES         (FCB_SIZE_IN_BYTES / DIR_ENTRY_SIZE)
#define MAX_FILE_SIZE           (1024*4096)
#define MAX_MAP_ENTRIES         256 //REVISAR
#define MAX_ROOT_DENTRIES       16  //REVISAR

const int FCB_SIZE_IN_BLOCKS = 5120;
const int SECTOR_SIZE2 = 4096;
const int DIR_ENTRY_SIZE = 512;
const long FCB_SIZE_IN_BYTES = 5120*4*1024;//FCB_SIZE_IN_BLOCKS * 4 * 1024;
const int FCB_MAX_ENTRIES = 40960;//(FCB_SIZE_IN_BYTES / DIR_ENTRY_SIZE);

struct dir_entry_old {
    char name[483];//[MAX_NAME_LEN];
    unsigned char sectors[MAX_SECTORS_PER_FILE];
};

struct file_info_old {
    struct dir_entry_old *f_dentry;
    int f_size;
    unsigned char f_data[MAX_FILE_SIZE];//[MAX_FILE_SIZE];
    int need_update;
};

struct file_info {
    struct dir_entry *f_dentry;
    //int f_size; in bytes
    int sectors[MAX_SECTORS_PER_FILE];
    unsigned char f_data[MAX_FILE_SIZE];
    int need_update;
};

struct dir_entry {
    char name[MAX_NAME_LEN];
    char type; // f=file, d=directory
    int  pointers_block;
    int  size_in_blocks;
    int  size_in_bytes;
    long created_on;
    long updated_on;
};

int sfs_getattr(const char *path, struct stat *statbuf);
int sfs_readlink(const char *path, char *link, size_t size);
int sfs_mknod(const char *path, mode_t mode, dev_t dev);
int sfs_mkdir(const char *path, mode_t mode);
int sfs_unlink(const char *path);
int sfs_rmdir(const char *path);
int sfs_symlink(const char *path, const char *link);
int sfs_rename(const char *path, const char *newpath);
int sfs_link(const char *path, const char *newpath);
int sfs_chmod(const char *path, mode_t mode);
int sfs_chown(const char *path, uid_t uid, gid_t gid);
int sfs_truncate(const char *path, off_t newSize);
int sfs_utime(const char *path, struct utimbuf *ubuf);
int sfs_open(const char *path, struct fuse_file_info *fileInfo);
int sfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo);
int sfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo);
int sfs_statfs(const char *path, struct statvfs *statInfo);
int sfs_flush(const char *path, struct fuse_file_info *fileInfo);
int sfs_release(const char *path, struct fuse_file_info *fileInfo);
int sfs_fsync(const char *path, int datasync, struct fuse_file_info *fi);
int sfs_opendir(const char *path, struct fuse_file_info *fileInfo);
int sfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo);
int sfs_releasedir(const char *path, struct fuse_file_info *fileInfo);
int sfs_fsyncdir(const char *path, int datasync, struct fuse_file_info *fileInfo);
void sfs_init(struct fuse_conn_info *conn);
void sfs_destroy(void *userdata);

#ifdef __cplusplus
}
#endif

#endif //sfs_h

