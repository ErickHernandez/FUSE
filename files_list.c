#include "files_list.h"

/* Name: Generic Linked List in C
 * Link: geeksforgeeks.org/generic-linked-list-in-c-2/
 * Genre: Linked list
 * Description:
 * */
 
list_node *new_node(char *name, long creation_date, long last_update, int size_in_blocks, int size_in_kb, char type, int pointers_block)
{

    dir_entry *de = (dir_entry *) malloc(sizeof(dir_entry));;
    if (de == NULL) return NULL;

    de->name = name;
    de->creation_date = creation_date;
    de->last_update = last_update;
    de->size_in_blocks = size_in_blocks;
    de->size_in_kb = size_in_kb;
    de->type = type;
    de->pointers_block = pointers_block;
    de->dir_entry_size = sizeof(dir_entry);
    
    list_node *tmp = (list_node *) malloc(sizeof(list_node));
    if (tmp == NULL) return NULL;     

    tmp->data = de;
    
    tmp->next = NULL;
    return tmp;
}

int add_node(list_node *l, char *name, long creation_date, long last_update, int size_in_blocks, int size_in_kb, char type, int pointers_block)
{    
    if (l == NULL) return 0;
    list_node *tmp = l;
    
    while (tmp != NULL) {

        if(strcmp(tmp->data->name, name) == 0 && tmp->data->type == type){ 
            return -1; //node already exists
            break;
        }

        if(tmp->next == NULL){
            tmp->next = new_node(name, creation_date, last_update, size_in_blocks, size_in_kb, type, pointers_block);
            break;
        }

        tmp = tmp->next;
    }

    return 1;
}

list_node* find_node(list_node *l, char *name)
{          
    if (l == NULL) return NULL;
    list_node *tmp = l;
    
    while (tmp != NULL) {

        if(strcmp(tmp->data->name, name) == 0){   
            return tmp;            
        }

        tmp = tmp->next;
    }

    return NULL;
}

//TODO: Agregar la lógica de borrado
int remove_node(list_node *l, char *name)
{
    if (l == NULL) return 0;
    while (l != NULL) {
        /*if(l->next == NULL)
            l->next = new_node(name, creation_date, last_update, size_in_blocks, size_in_kb, type, pointers_block);
        else
            l = l->next;*/
    }

    return 1;
}

void free_list(list_node **l)
{
    if (l == NULL) return ;
    list_node *next;    
    while (*l != NULL) {
        next = (*l)->next;        
        free((*l)->data);
        free(*l);
        *l   = next;
    }
}
 
void print_str(dir_entry *entry)
{ printf("%s", entry->name); }
 
void print_list(const list_node *l)
{
    if (l == NULL) return ;
    while (l != NULL) {
        print_str(l->data);
        l = l->next;
        putchar(l ? ' ' : '\n');
    }
}
 
list_node *build_list(char *name, long creation_date, long last_update, int size_in_blocks, int size_in_kb, char type, int pointers_block)
{
    list_node *head = new_node(name, creation_date, last_update, size_in_blocks, size_in_kb, type, pointers_block);
    return head;
}



/*int main(void)
{
    
    list_node *l_char  = build_list("file1", 123, 456, 1, 2, 'f', -1);
    add_node(l_char, "file2", 123, 456, 1, 2, 'f', -1);
    add_node(l_char, "file3", 123, 456, 1, 2, 'f', -1);
    add_node(l_char, "file4", 123, 456, 1, 2, 'f', -1);
    print_list(l_char);
    free_list(&l_char);
    return 0;
}*/