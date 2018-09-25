#ifndef _BITMAP_H
#define _BITMAP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    unsigned char* bits;    
    int free_blocks;
    int size_in_blocks;
    int size_in_bits;
}bitvector;

bitvector* create_bitvec(int size_gb, int block_size_kb);//size in Gigabytes
int get_bit(bitvector* vec, int index);
void set_bit(bitvector* vec, int index, int bit);
int *get_free_blocks(bitvector* vec, int amount);
void update_free_blocks(bitvector *vec, int new_amount);
void print(bitvector* vec);
void destroy_bitvec(bitvector *vec);

#ifdef __cplusplus
}
#endif

#endif