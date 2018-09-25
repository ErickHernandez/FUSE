#include "bitmap.h"
#include "math.h"

bitvector* build_bitmap(int size_in_bits, int size_in_blocks) {
    
    bitvector* vec = (bitvector*)malloc(sizeof(bitvector));
    vec->size_in_bits = size_in_bits;
    vec->size_in_blocks = size_in_blocks;
    vec->free_blocks = size_in_bits - size_in_blocks;
    int size = sizeof(bitvector) * (size_in_bits/8.0); //size in bytes    
    vec->bits = (unsigned char*)malloc(size);

    int i;
    for(i=0; i<vec->size_in_blocks; i++)
    {
        set_bit(vec, i, 1);
    }

    return vec;
}

void destroy_bitvec(bitvector *vec) {
    free(vec->bits);
    free(vec);
}

//Calculate the amount of bits needed for the bitmap, considering blocks of 4kb each.
int bitmap_size_in_bits(int size_gb, int block_size_kb)
{
    //TODO: Validate values with decimal points
    return ((ldexp(size_gb, 30) / ldexp(block_size_kb, 10)));
}

int bitmap_size_in_blocks(int bits, int block_size_kb)
{
    return (bits/8/1024/block_size_kb);
}

void set_bit(bitvector* vec, int index, int bit) {
    int byte = index >> 3;
    int n = sizeof(index)*8-3; //29
    int offset = ((unsigned) index << n) >> n;
    if (bit) {
        vec->bits[byte] |= 1 << (7-offset);
        vec->free_blocks -= 1;
    } else {
        vec->bits[byte] &= ~(1 << (7-offset));
        vec->free_blocks += 1;
    }
}

int get_bit(bitvector* vec, int index) {
    int byte = index >> 3;
    int n = sizeof(index)*8-3;
    int offset = ((unsigned) index << n) >> n;
    if (vec->bits[byte] & (1 << (7- offset))) {
        return 1;
    } else {
        return 0;
    }
}

//size in gigabytes
bitvector* create_bitvec(int size_gb, int block_size_kb)
{
    int size_in_blocks;
    int size_in_bits;

    size_in_bits = bitmap_size_in_bits(size_gb, block_size_kb);
    size_in_blocks = bitmap_size_in_blocks(size_in_bits, block_size_kb);

    bitvector *vec = build_bitmap(size_in_bits, size_in_blocks);

    return vec;
}

void update_free_blocks(bitvector *vec, int new_amount)
{
    vec->free_blocks = new_amount;
}


void print(bitvector* vec)
{ 
    int i = 0;
    
    for (i = 0; i < vec->size_in_bits; i++) {
        printf("%d", get_bit(vec, i));
    }
}

int *get_free_blocks(bitvector* vec, int amount)
{
    int *free_blocks = (int*)malloc(sizeof(int)*amount);
    memset(free_blocks, 0, sizeof(int)*amount);

    int i, j=0;

    if(amount <= vec->free_blocks)
    {                
        
        for (i=0; i < vec->size_in_bits; i++) 
        {
            if(get_bit(vec, i) == 0)
            {                
                free_blocks[j++] = i;
                
                if(j == amount)
                    return free_blocks;
            }
        }
    }

    free(free_blocks);
    return NULL;
}

/*int main ()
{
    int gb = 1024;
    int block_size_kb = 4;
    int bits = bitmap_size_in_bits(gb, block_size_kb);

    printf("bits %d\n", bits);
    printf("blocks %d\n", bitmap_size_in_blocks(bits, block_size_kb));
}*/
    