#ifndef _FILES_LIST_H_
#define _FILES_LIST_H_

#define MAX_NAME_LEN		        500


#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct dir_entry{
    char *name;
    long creation_date;
    long last_update;
    int size_in_blocks;
    int size_in_kb;    
    char type;
    int pointers_block;
    size_t dir_entry_size;
} dir_entry;

typedef struct list_node {
    dir_entry *data;
    struct list_node *next;
} list_node;

int add_node(list_node *l, char *name, long creation_date, long last_update, int size_in_blocks, int size_in_kb, char type, int pointers_block);
list_node *new_node(char *name, long creation_date, long last_update, int size_in_blocks, int size_in_kb, char type, int pointers_block);
int remove_node(list_node *l, char *name);
void free_list(list_node **l);
void print_list(const list_node *l);
list_node *build_list(char *name, long creation_date, long last_update, int size_in_blocks, int size_in_kb, char type, int pointers_block);
list_node* find_node(list_node *l, char *name);

#ifdef __cplusplus
}
#endif

#endif