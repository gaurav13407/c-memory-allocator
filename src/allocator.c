#include <unistd.h>
#include <stddef.h>


// Block MetaData
 
typedef struct block{
    size_t size;
    int free;
    struct block *next;
} block_t;

#define BLOCK_SIZE sizeof(block_t)

static block_t *head_heap=NULL;

//---------------Find Free Block-------------------- 
 
static block_t *find_free_block(size_t size){
    block_t *current=head_heap;

    while(current){
        if(current->free && current->size>=size){
            return current;
        }
        current=current->next;
    }
    return NULL;
}

//Request Memory from os----------------------

static block_t *request_memory(size_t size){
    void *request=sbrk(0);
    void *allocate=sbrk(size+BLOCK_SIZE);

    if(allocate==(void *)-1){
        return NULL;
    }

    block_t *block=(block_t *)request;
    block->size=size;
    block->free=0;
    block->next=NULL;

    return block;
}

//-------------------malloc Implementation-------------------------

void *my_malloc(size_t size){
    if (size==0){
        return NULL;
    }

    block_t *block;
    if(!head_heap){
        block=request_memory(size);
        if(!block)
            return NULL;
        head_heap=block;
    }else{
        block=find_free_block(size);
        if(!block){
            block_t *last=head_heap;
            while(last->next){
                last=last->next;
            }
            block=request_memory(size);
            if(!block)
                return NULL;

            last->next=block;
        }else{
            block->free=0;
        }
    }

    return (void *)(block+1);
}

//-------------------free Implementation------------------------- 

void my_free(void *ptr){
    if(!ptr)
        return;
    block_t *block=(block_t *)ptr-1;
    block->free=1;
}
