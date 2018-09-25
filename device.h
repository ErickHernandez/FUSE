#ifndef _DEVICE_H_
#define _DEVICE_H_

#define SECTOR_SIZE		        4096

#ifdef __cplusplus
extern "C" {
#endif

#include "bitmap.h"

int device_open(const char *path);
void device_close();
int device_read_sector(unsigned char buffer[], int sector);
int device_write_sector(unsigned char buffer[], int sector);
int device_read_map(bitvector *bitmap);
int device_write_map(bitvector *bitmap);
int device_read_FCB(unsigned char buffer[], int blocks);
int device_write_FCB(unsigned char buffer[], int blocks);
void device_flush();

#ifdef __cplusplus
}
#endif

#endif
