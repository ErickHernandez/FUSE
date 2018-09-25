
#include "bitmap.h"
#include "sfs.h"
#include "device.h"

unsigned char map[SECTOR_SIZE];
unsigned char root_dir_block[SECTOR_SIZE];
int root_dir_need_update, FCB_loaded = 0;
struct dir_entry_old *root_dir = (struct dir_entry_old *)root_dir_block;

struct dir_entry FCB2[FCB_MAX_ENTRIES];

bitvector *bitmap;

int sfs_load_FCB()
{
    printf("%s\n", __FUNCTION__);

    int i, j, pos=0;
    for(i=0; i<FCB_SIZE_IN_BLOCKS; i++){
        unsigned char FCB_buffer[SECTOR_SIZE];
        memset(FCB_buffer, 0, SECTOR_SIZE);        
        
        device_read_sector(FCB_buffer, i);        
        
        for(j=0; j<SECTOR_SIZE; j+=DIR_ENTRY_SIZE){

            //if(FCB_buffer[j] != '\0'){

                //struct dir_entry *de = &FCB2[pos++];
                //memset(de, 0, sizeof(struct dir_entry));

                //de = (struct dir_entry *)&FCB_buffer[j];
                memcpy(&FCB2[pos++], &FCB_buffer[j], sizeof(struct dir_entry));               
            //}
        }
    }

    FCB_loaded = 1;

    printf("sfs_load_FCB loaded ...\n");

    return 1;
}

void dump_hex(const void* data, size_t size) {
	char ascii[17];
	size_t i, j;
	ascii[16] = '\0';
	for (i = 0; i < size; ++i) {
		printf("%02X ", ((unsigned char*)data)[i]);
		if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
			ascii[i % 16] = ((unsigned char*)data)[i];
		} else {
			ascii[i % 16] = '.';
		}
		if ((i+1) % 8 == 0 || i+1 == size) {
			printf(" ");
			if ((i+1) % 16 == 0) {
				printf("|  %s \n", ascii);
			} else if (i+1 == size) {
				ascii[(i+1) % 16] = '\0';
				if ((i+1) % 16 <= 8) {
					printf(" ");
				}
				for (j = (i+1) % 16; j < 16; ++j) {
					printf("   ");
				}
				printf("|  %s \n", ascii);
			}
		}
	}
}

void sfs_ensure_FCB_loaded()
{
    if (!FCB_loaded) {
        device_read_sector(root_dir_block, 2);
        
        FCB_loaded = 1;
        
        printf("Loading root directory ...\n");
        dump_hex(root_dir_block, 64);
    }
}

void sfs2_ensure_FCB_loaded()
{
    printf("%s\n", __FUNCTION__);

    if (!FCB_loaded) {
        printf("FCB will be loaded %s\n", __FUNCTION__);
               
        FCB_loaded = sfs_load_FCB();
        
        printf("Loading root directory ...\n");
        //dump_hex(root_dir_block, 64);
    }
}

struct dir_entry_old *sfs_lookup_root_dir(const char *d_name) 
{
    int i = 0;

    sfs_ensure_FCB_loaded();
    while (i<MAX_ROOT_DENTRIES) {
        
        if (root_dir[i].name[0] != '\0') {
            if (strncmp(d_name, root_dir[i].name, MAX_NAME_LEN) == 0)
                return &root_dir[i];
        }
        
        i++;
    }
    
    return NULL;
}

//EH: Done
struct dir_entry *sfs2_lookup_root_dir(const char *d_name)//, const char type) 
{
    printf("%s: %s\n", __FUNCTION__, d_name);
    sfs2_ensure_FCB_loaded();

    int i = 0;
    
    while (i<FCB_MAX_ENTRIES) {
        
        if (FCB2[i].name[0] != '\0') {
            if (strncmp(d_name, FCB2[i].name, MAX_NAME_LEN) == 0)
                return &FCB2[i];
        }
        
        i++;
    }
    
    return NULL;
}

void sfs_get_file_stat(struct dir_entry_old *dentry, int *size, int *block_count)
{
    int i;
    
    *size = 0;
    *block_count = 0;
    
    for (i = 0; dentry->sectors[i] != 0; i++) {
        (*size) += SECTOR_SIZE;
        (*block_count)++;
    }
}

//EH
struct file_info *sfs_load_file(struct dir_entry *dentry)
{
    struct file_info *fi = (struct file_info *)malloc(sizeof(struct file_info));
    unsigned char *ptr_data = fi->f_data;
    
    if (fi == NULL)
        return NULL;
    
    int i, size = 0;
    
    memset(fi, 0, sizeof(struct file_info));

    if(dentry->pointers_block != 0){
       
        unsigned char buffer[SECTOR_SIZE];
        memset(buffer, 0, SECTOR_SIZE);
        //int sectors[MAX_SECTORS_PER_FILE];
        device_read_sector(buffer, dentry->pointers_block);

        //if(buffer[0] != '\0'){

            int *sectors = fi->sectors;
            memset(sectors, 0, sizeof(int)*MAX_SECTORS_PER_FILE);
            sectors = (int*)buffer;

            for(i=0; i<MAX_SECTORS_PER_FILE; i++){

                if(sectors[i] != 0){
                    device_read_sector(ptr_data, sectors[i]);

                    size += SECTOR_SIZE;
                    ptr_data += SECTOR_SIZE;
                }
            }

        //}
    }
    
    fi->f_dentry = dentry;
    fi->f_dentry->size_in_bytes = size;
    fi->f_dentry->size_in_blocks = i;
    
    return fi;
}
void sfs_update_map()
{
    printf("Writing bitmap to disc ...\n");
    
    device_write_map(bitmap);
    device_flush();

    printf("bitmap Updated ...\n");
}

void sfs_update_FCB()
{
    printf("%s\n", __FUNCTION__);
    printf("Writing FCB to disc ...\n");

    /*int i = 0, sector = 0;    

    char buffer[SECTOR_SIZE];
    memset(buffer, 0, SECTOR_SIZE);

    while (i<FCB_MAX_ENTRIES) {
        
        if(i % (SECTOR_SIZE/DIR_ENTRY_SIZE) == 0){            
        
            device_write_sector((unsigned char*)buffer, sector++);

            //CleanBuffer
            memset(buffer, 0, SECTOR_SIZE);            
        }
    
        strcat(buffer, (const char*)&FCB2[i]);
        i++;
    }

    //if buffer is not empty, there is dir_entries remaining to write
    if(buffer[0] != '\0'){
        device_write_sector((unsigned char*)buffer, sector);
    }*/

    device_write_FCB((unsigned char*)FCB2, FCB_SIZE_IN_BLOCKS);

    device_flush();

    printf("FCB Updated ...\n");
}

void sfs_remove_file(struct dir_entry *dentry)
{
    int i;

    if(dentry->pointers_block != 0){
       
        unsigned char buffer[SECTOR_SIZE];
        memset(buffer, 0, SECTOR_SIZE);        
        device_read_sector(buffer, dentry->pointers_block);
        set_bit(bitmap, dentry->pointers_block, 0);
        

        int *sectors = (int*)buffer;

        for(i=0; i<MAX_SECTORS_PER_FILE; i++){

            if(sectors[i] != 0){
                set_bit(bitmap, sectors[i], 0);
            }
        }

        sfs_update_map();
    }

    memset(dentry, 0, sizeof(struct dir_entry));
    sfs_update_FCB();
}

int sfs_get_free_block()
{
    int i = 0;
    
    while ( (map[i] != 0) && (i < MAX_MAP_ENTRIES) )
        i++;
    
    if (i < 3) {
        printf("BUG in the machine, sectors 0, 1, 2 cannot be used to store file data\n" );
        dump_hex(map, 16);
        return -1;
    }

    return (map[i] != 0)? -1 : i;
}

//EH
int sfs_commit_file(struct file_info *fi)
{

    printf("commit file");

    struct dir_entry *de = fi->f_dentry;
    char f_name[MAX_NAME_LEN+1];
    int i;
        
    printf("%s: size=%d\n", __FUNCTION__, fi->f_dentry->size_in_bytes);
    
    strncpy(f_name, de->name, MAX_NAME_LEN);
    f_name[MAX_NAME_LEN] = 0;
    
    unsigned char *ptr_data = fi->f_data;
    
    for (i = 0; MAX_SECTORS_PER_FILE; i++) {

        if(fi->sectors[i] != 0){
            int sector = fi->sectors[i];
        
            printf("File '%s' writing sector %d\n", f_name, sector);
            device_write_sector(ptr_data, sector);
            
            ptr_data += SECTOR_SIZE;
        }        
    }
    
    device_flush();
    
    return 1;
}

void sfs_get_root_dir_info(int *file_count, int *free_dentries)
{
    int i = 0;

    *file_count = 0;
    *free_dentries = 0;
    
    sfs_ensure_FCB_loaded();
        
    while (i<MAX_ROOT_DENTRIES) {
        
        if (root_dir[i].name[0] != '\0') {
            (*file_count)++;
        } else {
            (*free_dentries)++;
        }
        
        i++;
    }
    
    printf("File count: %d Free entries: %d\n", *file_count, *free_dentries);
}

int sfs_count_free_blocks()
{
    int i, free_block_count;
    
    free_block_count = 0;
    for (i=0; i<MAX_MAP_ENTRIES; i++)  {
        if (map[i] == 0x0) free_block_count++;
    }
    
    return free_block_count;
}

//EH
int sfs_getattr(const char *path, struct stat *statbuf) 
{
    int path_len = strlen(path);
    printf("%s: %s\n", __FUNCTION__, path);
    
    if ( (path_len == 1) && path[0] == '/') {
        statbuf->st_mode = S_IFDIR | S_IRWXU | S_IRWXG | S_IRWXO;
        statbuf->st_uid = 0;
        statbuf->st_gid = 0;
        statbuf->st_nlink = 1;
        statbuf->st_ino = 0;
        statbuf->st_size = SECTOR_SIZE;
        statbuf->st_blksize = SECTOR_SIZE;
        statbuf->st_blocks = 1;        
    } else {              
        struct dir_entry *dentry = sfs2_lookup_root_dir(&path[1]);
        printf("Enter else - %s: %s\n", __FUNCTION__, &path[1]);
        int size, block_count;
        
        if (dentry == NULL){
            printf("dentry is NULL - %s: %s\n", __FUNCTION__, &path[1]);
            return -ENOENT;
        }        
        
        //sfs_get_file_stat(dentry, &size, &block_count);
        size = dentry->size_in_blocks*SECTOR_SIZE;
        block_count = dentry->size_in_blocks;

        if(dentry->type == 'f'){
            statbuf->st_mode = S_IFREG | 0777; //(S_IFREG | S_IRWXU | S_IRWXG | S_IRWXO) & ~S_IXUSR & ~S_IXGRP & ~S_IXOTH;
            statbuf->st_nlink = 1;
        }else if(dentry->type == 'd'){
            statbuf->st_mode = S_IFDIR | 0755;
            statbuf->st_nlink = 2;
        }
        
        //statbuf->st_mode = (S_IFREG | S_IRWXU | S_IRWXG | S_IRWXO) & ~S_IXUSR & ~S_IXGRP & ~S_IXOTH;
        //statbuf->st_nlink = 1;
        statbuf->st_size = size;
        statbuf->st_ino = 0;//(ino_t)(dentry - 0);
        statbuf->st_uid = 0;
        statbuf->st_gid = 0;        
        statbuf->st_blksize = SECTOR_SIZE;              /* blocksize for filesystem I/O */
        statbuf->st_blocks = block_count;               /* number of 4096B blocks allocated */
    }
    
    return 0;
}

int sfs_readlink(const char *path, char *link, size_t size) {
    return -EPERM;
}

//EH
int sfs_mknod(const char *path, mode_t mode, dev_t dev) 
{
    printf("mknod(path=%s, mode=%d)\n", path, mode);
    
    if ( S_ISREG(mode) ) {
        int i;
        char f_name[MAX_NAME_LEN+1];
        const char *of_name = &path[1]; //Removing the / from the beggining of the path
        struct dir_entry de;
        
        memset(f_name, 0, MAX_NAME_LEN+1);
        strncpy(f_name, of_name, MAX_NAME_LEN);
        f_name[MAX_NAME_LEN] = 0;
        
        struct dir_entry *dire = sfs2_lookup_root_dir(f_name);
        if (dire != NULL)
            return -EEXIST;
        
        printf("Creating file %s\n", f_name);
        
        sfs2_ensure_FCB_loaded();
                
        //Look up a empty directory entry
        i = 0;
        while (FCB2[i].name[0] != '\0' && i<FCB_MAX_ENTRIES) i++;
        
        if (FCB2[i].name[0] != '\0')
            return -ENOSPC;

        int *pointers = get_free_blocks(bitmap, 1);        

        if(pointers == NULL)
            return -ENOSPC;        
        
        set_bit(bitmap, pointers[0], 1);
        //update_free_blocks(bitmap, bitmap->free_blocks - 1);

        //seek to the first data block
        pointers[0] = FCB_SIZE_IN_BLOCKS + bitmap->size_in_blocks + pointers[0];

        printf("Creating file with inode %d and pointers_block %d\n", i, pointers[0]);        
                
        memcpy(de.name, f_name, MAX_NAME_LEN);
        de.type = 'f';
        de.pointers_block = pointers[0];
        de.size_in_blocks = 1;
        de.size_in_bytes = 0;       

        time_t now = time(NULL);
        if (now != -1){
            de.created_on = now;
            de.updated_on = now;
        }

        FCB2[i] = de;

        printf("FCB with file %s\n", f_name);
        
        sfs_update_FCB();
        sfs_update_map();//Agregado por mi
        
        return 0;
    }
    return -EPERM;
}

int sfs_mkdir(const char *path, mode_t mode) {
    printf("**mkdir(path=%s, mode=%d)\n", path, (int)mode);    
    
    //if ( S_ISREG(mode) ) {
        int i;
        char f_name[MAX_NAME_LEN+1];
        const char *of_name = &path[1]; //Removing the / from the beggining of the path
        struct dir_entry de;
        
        memset(f_name, 0, MAX_NAME_LEN+1);
        strncpy(f_name, of_name, MAX_NAME_LEN);
        f_name[MAX_NAME_LEN] = 0;
        
        struct dir_entry *dire = sfs2_lookup_root_dir(f_name);
        if (dire != NULL)
            return -EEXIST;
        
        printf("Creating dir %s\n", f_name);
        
        sfs2_ensure_FCB_loaded();
                
        //Look up a empty directory entry
        i = 0;
        while (FCB2[i].name[0] != '\0' && i<FCB_MAX_ENTRIES) i++;
        
        if (FCB2[i].name[0] != '\0')
            return -ENOSPC;

        //int *pointers = get_free_blocks(bitmap, 1);        

        //if(pointers == NULL)
            //return -ENOSPC;        
        
        //set_bit(bitmap, pointers[0], 1);

        //printf("Creating file with inode %d and pointers_block %d\n", i, pointers[0]);
        printf("Creating dir with inode %d\n", i);        
                
        memcpy(de.name, f_name, MAX_NAME_LEN);
        de.type = 'd';
        de.pointers_block = 0;
        de.size_in_blocks = 0;
        de.size_in_bytes = 0;

        time_t now = time(NULL);
        if (now != -1){
            de.created_on = now;
            de.updated_on = now;
        }

        FCB2[i] = de;

        printf("FCB with dir %s\n", f_name);
        
        sfs_update_FCB();
        sfs_update_map();//Agregado por mi
        
        return 0;
    /*}

    return -EPERM;*/
}

//EH
int sfs_unlink(const char *path) 
{
    struct dir_entry *de = sfs2_lookup_root_dir(&path[1]);
    
    printf("unlink(path=%s)\n", path);
    
    if (de == NULL)
        return -ENOENT;
    
    sfs_remove_file(de);
    
    return 0;
}

int sfs_rmdir(const char *path) {
    printf("rmdir(path=%s\n)", path);    

    int i=0;
    while (strncmp(FCB2[i].name, &path[1], MAX_NAME_LEN) != 0 && i<FCB_MAX_ENTRIES) i++;

    if(strncmp(FCB2[i].name, &path[1], MAX_NAME_LEN) == 0){
        struct dir_entry empty;
        FCB2[i] = empty;

        sfs_update_FCB();

        return 0;
    }

    return -ENOENT;
}

int sfs_symlink(const char *path, const char *link) {
    printf("symlink(path=%s, link=%s)\n", path, link);
    return -EPERM;
}

int sfs_rename(const char *path, const char *newpath) 
{
    struct dir_entry *de = sfs2_lookup_root_dir(&path[1]);
    
    printf("rename(path=%s, newPath=%s)\n", path, newpath);
    if (de == NULL)
        return -ENOENT;
    
    strncpy(de->name, &newpath[1], MAX_NAME_LEN);
    sfs_update_FCB();
    
    return 0;
}

int sfs_link(const char *path, const char *newpath) {
	printf("link(path=%s, newPath=%s)\n", path, newpath);
    return -EPERM;
}

int sfs_chmod(const char *path, mode_t mode) {
	printf("chmod(path=%s, mode=%d)\n", path, mode);
    return -EPERM;
}

int sfs_chown(const char *path, uid_t uid, gid_t gid) {
	printf("chown(path=%s, uid=%d, gid=%d)\n", path, (int)uid, (int)gid);
    return -EPERM;
}

int sfs_truncate(const char *path, off_t newSize) 
{
	struct dir_entry_old *de = sfs_lookup_root_dir(&path[1]);
    
    printf("truncate(path=%s, newSize=%d)\n", path, (int)newSize);
    
    if (de == NULL)
        return -ENOENT;
    
    if (newSize == 0) {
        int i;
        
        for (i=0; de->sectors[i] != 0; i++) {
            int sector = de->sectors[i];
            de->sectors[i] = 0;
            map[sector] = 0;
            printf("Releasing sector %d\n", sector);
        }
        sfs_update_map();
    }
    
    return 0;
}

int sfs_utime(const char *path, struct utimbuf *ubuf) {
	printf("utime(path=%s)\n", path);
    return 0;
}

//EH
int sfs_open(const char *path, struct fuse_file_info *fileInfo) 
{
    printf("open(path=%s)\n", path);
    
    //Remove first character because files doesn't have / in the beggining
    struct dir_entry *de = sfs2_lookup_root_dir(&path[1]);
    
    if (de == NULL)
        return -ENOENT;

    printf("path=%s found!\n", path);

    struct file_info *fi = sfs_load_file(de);
    
    if (fi == NULL)
        return -ENOMEM;
    
    fileInfo->fh = (uint64_t)fi;
    fi->need_update = 0;
    
    return 0;
}

int sfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo) 
{
    struct file_info_old *fi = (struct file_info_old *)fileInfo->fh;
    int bytes_to_read = size;

    printf("read(path=%s, size=%d, offset=%d, f_size=%d)\n", path, (int)size, (int)offset, fi->f_size);
    
    if (offset >= fi->f_size)
        return 0;
    
    if (offset + size > fi->f_size)
        bytes_to_read = fi->f_size - offset;
    
    memcpy(buf, &fi->f_data[offset], size);
    
    return bytes_to_read;
}

int sfs_grow_file(struct file_info *fi, int new_size)
{
    struct dir_entry *de = fi->f_dentry;
    int i, extra_size;


    /*

    for(i=0; i<MAX_SECTORS_PER_FILE; i++){

        if(sectors[i] != 0){
            device_read_sector(ptr_data, sectors[i]);

            size += SECTOR_SIZE;
            ptr_data += SECTOR_SIZE;
        }
    }
    */

    unsigned char buffer[SECTOR_SIZE];
    memset(buffer, 0, SECTOR_SIZE);
    device_read_sector(buffer, de->pointers_block);
    int *sectors = fi->sectors; 
    memset(sectors, 0, sizeof(int)*MAX_SECTORS_PER_FILE);
    sectors = (int*)buffer;
    
    for (i = 0; sectors[i] != 0; i++);
    
    printf("Growing file, size = %d, new size = %d\n", fi->f_dentry->size_in_bytes, new_size);
    //dump_hex(de->name, 6);
    printf("\n");
    
    extra_size = new_size - fi->f_dentry->size_in_bytes;
    while (extra_size > 0) {
        
        int *pointers = get_free_blocks(bitmap, 1);        

        if(pointers == NULL)
            return -ENOSPC;  
        
        int sector = pointers[0];

        set_bit(bitmap, sector, 1);

        fi->sectors[i] = sector;
        extra_size -= SECTOR_SIZE;
        i++;
        
        printf("Allocating block %d\n", sector);
        if (i >= MAX_SECTORS_PER_FILE)
            return -ENOSPC;
    }
    
    sfs_update_FCB();
    sfs_update_map();
    
    return 0; 
}

/*
int sfs_grow_file(struct file_info_old *fi, int new_size)
{
    struct dir_entry_old *de = fi->f_dentry;
    int i, extra_size;
    
    for (i = 0; de->sectors[i] != 0; i++);
    
    printf("Growing file, size = %d, new size = %d\n", fi->f_size, new_size);
    dump_hex(de->name, 6);
    printf("\n");
    
    extra_size = new_size - fi->f_size;
    while (extra_size > 0) {
        int sector = sfs_get_free_block();
        if (sector == -1) {
            return -ENOSPC;
        }
        map[sector] = 0xFF;
        de->sectors[i] = sector;
        extra_size -= SECTOR_SIZE;
        i++;
        
        printf("Allocating block %d\n", sector);
        if (i >= MAX_SECTORS_PER_FILE)
            return -ENOSPC;
    }
    
    sfs_update_FCB();
    sfs_update_map();
    
    return 0; 
}
*/

int sfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo) 
{
    struct file_info *fi = (struct file_info *)fileInfo->fh;
    int f_size, f_bcount;

    //sfs_get_file_stat(fi->f_dentry, &f_size, &f_bcount);    

    f_size = fi->f_dentry->size_in_bytes;
    f_bcount = fi->f_dentry->size_in_blocks;

    printf("write(path=%s, size=%d, offset=%d, f_size1=%d, f_size2=%d)\n", path, (int)size, (int)offset, fi->f_dentry->size_in_bytes, f_size);

    //fi->f_size = f_size;
    if (offset + size > f_size) {
        if (offset + size > MAX_FILE_SIZE)
            return -EFBIG;
        
        int new_size = (offset + size);
        int ret = sfs_grow_file(fi, new_size);//TODO - GROWFILE
        
        if (ret < 0)
            return ret;       
        
        fi->f_dentry->size_in_bytes = new_size;
    }

    fi->need_update = 1;
    memcpy(&fi->f_data[offset], buf, size);
    
    printf("After write(path=%s, size=%d, offset=%d, f_size=%d)\n", path, (int)size, (int)offset, fi->f_dentry->size_in_bytes);

    return size;
}

/*
int sfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo) 
{
    struct file_info_old *fi = (struct file_info_old *)fileInfo->fh;
    int f_size, f_bcount;

    sfs_get_file_stat(fi->f_dentry, &f_size, &f_bcount);
    printf("write(path=%s, size=%d, offset=%d, f_size1=%d, f_size2=%d)\n", path, (int)size, (int)offset, fi->f_size, f_size);

    fi->f_size = f_size;
    if (offset + size > fi->f_size) {
        if (offset + size > MAX_FILE_SIZE)
            return -EFBIG;
        
        int new_size = (offset + size);
        int ret = sfs_grow_file(fi, new_size);
        
        if (ret < 0)
            return ret;       
        
        fi->f_size = new_size;
    }

    fi->need_update = 1;
    memcpy(&fi->f_data[offset], buf, size);
    
    printf("After write(path=%s, size=%d, offset=%d, f_size=%d)\n", path, (int)size, (int)offset, fi->f_size);

    return size;
}
*/

int sfs_statfs(const char *path, struct statvfs *statInfo) 
{
	printf("statfs(path=%s)\n", path);
    int file_count, free_dentries;
    
    sfs_get_root_dir_info(&file_count, &free_dentries);
    
    statInfo->f_bsize = SECTOR_SIZE;    /* filesystem block size */
    statInfo->f_frsize = SECTOR_SIZE;   /* fragment size */
    statInfo->f_blocks = 2880;          /* size of fs in f_frsize units */
    statInfo->f_bfree = sfs_count_free_blocks();    /* # free blocks */
    statInfo->f_bavail = sfs_count_free_blocks();   /* # free blocks for unprivileged users */
    statInfo->f_files = MAX_ROOT_DENTRIES;          /* # inodes */
    statInfo->f_ffree = free_dentries;              /* # free inodes */
    statInfo->f_favail = free_dentries;             /* # free inodes for unprivileged users */
    statInfo->f_fsid = 0;                   /* filesystem ID */
    statInfo->f_flag = 0;                   /* mount flags */
    statInfo->f_namemax = MAX_NAME_LEN;     /* maximum filename length */
    
    return 0;
}

int sfs_flush(const char *path, struct fuse_file_info *fileInfo) 
{
    //int f_size, f_bcount;
    struct file_info *fi = (struct file_info *)fileInfo->fh;
    
    printf("flush(path=%s)\n", path);
    
    //sfs_get_file_stat(fi->f_dentry, &f_size, &f_bcount);
    //f_size = fi->f_dentry->size_in_bytes;
    //f_bcount = fi->f_dentry->size_in_blocks;
    
    if (fi->need_update){// || (f_size != fi->f_size) ) {
        
        //fi->f_size = f_size;
        if (!sfs_commit_file(fi))
            return -ENOSPC;
    }

    return 0;
}

int sfs_release(const char *path, struct fuse_file_info *fileInfo) 
{
    struct file_info *fi = (struct file_info *)fileInfo->fh;
    
    printf("release(path=%s)\n", path);
    
    free(fi);
    fileInfo->fh = 0;
    
    return 0;
}

int sfs_fsync(const char *path, int datasync, struct fuse_file_info *fi) {
	printf("fsync(path=%s, datasync=%d\n", path, datasync);
    return 0;
}

//EH
int sfs_opendir(const char *path, struct fuse_file_info *fileInfo) 
{
    int path_len = strlen(path);

    printf("%s: %s\n", __FUNCTION__, path);
    
    if (FCB_loaded == 1)
        return 0;
    
    printf("EH_log before  if");

    if ((path_len != 1) && path[0] != '/')
        return -EPERM;

    printf("EH_log before sfs_load_FCB");
    
    if (!sfs_load_FCB())
        return -ENOENT;
    
    return 0;    
}

//EH
int sfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo) {
    int i = 0;
    char name[MAX_NAME_LEN+1];
    
    printf("readdir(path=%s, offset=%d)\n", path, (int)offset);
    
    //TODO: Aqui es donde necesitare que el value del FCB sea una lista de files

    while (i<FCB_MAX_ENTRIES){

        if (FCB2[i].name[0] != '\0') {

            //if(FCB2[i].type == 'd'){
                strncpy(name, FCB2[i].name, MAX_NAME_LEN);
                name[MAX_NAME_LEN] = '\0';
            
                if( filler(buf, name, NULL, 0) != 0) 
                    return -ENOMEM;
            //}
        }

        i++;
    }
    
    return 0;
}

int sfs_releasedir(const char *path, struct fuse_file_info *fileInfo) {
    //int path_len = strlen(path);

    printf("%s: %s\n", __FUNCTION__, path);
    
    return 0;
}

int sfs_fsyncdir(const char *path, int datasync, struct fuse_file_info *fileInfo) {
    printf("%s: %s\n", __FUNCTION__, path);
    return -EPERM;
}

//EH
void sfs_init(struct fuse_conn_info *conn) 
{
    printf("Loading file system map ...\n");    
    
    //Initializing Bitmap
    bitmap = create_bitvec(4, 4);

    printf("Created system map ...\n");
    device_read_map(bitmap);

    printf("Read system map ...\n");

    //Initializing FCB    
    //memset(FCB2, 0, FCB_MAX_ENTRIES*DIR_ENTRY_SIZE);
    sfs_load_FCB();

    printf("Created File Table ...\n");
    //sfs_load_FCB();
}

//EH
void sfs_destroy(void *userdata)
{
    sfs_update_map();
    sfs_update_FCB();
    destroy_bitvec(bitmap);
}