#include <stdlib.h>
#include <stdio.h>
#include "device.h"

static const char *device_path;
static FILE *f;

int device_open(const char *path) 
{
    device_path = path;
    f = fopen(path, "r+");
	
    return (f != NULL);
}

void device_close()
{
    fflush(f);
    fclose(f);
}

int device_read_sector(unsigned char buffer[], int sector) 
{
    fseek(f, sector*SECTOR_SIZE, SEEK_SET);
	
    return ( fread(buffer, 1, SECTOR_SIZE, f) == SECTOR_SIZE );
}

int device_write_sector(unsigned char buffer[], int sector) 
{
    fseek(f, sector*SECTOR_SIZE, SEEK_SET);
	
    return ( fwrite(buffer, 1, SECTOR_SIZE, f) == SECTOR_SIZE );
}

int device_read_map(bitvector *bitmap)
{
    //5120 is the FCB <20Mb> size in blocks (20Mb = 20*1024/4 blocks)
    fseek(f, 5120, SEEK_SET);
	
    return ( fread(bitmap->bits, SECTOR_SIZE, bitmap->size_in_blocks, f) == bitmap->size_in_blocks );
}

int device_write_map(bitvector *bitmap)
{
    //5120 is the FCB <20Mb> size in blocks (20Mb = 20*1024/4 blocks)
    fseek(f, 5120, SEEK_SET);
	
    return ( fwrite(bitmap->bits, SECTOR_SIZE, bitmap->size_in_blocks, f) == bitmap->size_in_blocks );    
}

int device_read_FCB(unsigned char buffer[], int blocks)
{    
    fseek(f, 0, SEEK_SET);
	
    return ( fread(buffer, SECTOR_SIZE, blocks, f) == blocks );
}

int device_write_FCB(unsigned char buffer[], int blocks)
{    
    fseek(f, 0, SEEK_SET);
	
    return ( fwrite(buffer, SECTOR_SIZE, blocks, f) == blocks );
}

void device_flush()
{
    fflush(f);
}
