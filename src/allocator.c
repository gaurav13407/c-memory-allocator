#include "allocator.h"
#include <unistd.h>
#include <stddef.h>
#include <stdalign.h>

// Block MetaData
 
typedef struct block{
    struct{
    size_t size;
    int free;
    struct block *next;
    } meta;

    max_align_t _align;
} block_t;

static block_t *head_heap=NULL;
#define ALIGNMENT ALLOCATOR_ALIGNMENT
#define ALIGN(size)\
    (((size)+(ALIGNMENT-1))& ~(ALIGNMENT-1))


#define BLOCK_SIZE ALIGN(sizeof(block_t))

//---------------Find Free Block-------------------- 
 
static block_t *find_free_block(size_t size){
    block_t *current=head_heap;

    while(current){
        if(current->meta.free && current->meta.size>=size){
            return current;
        }
        current=current->meta.next;
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
    block->meta.size=size;
    block->meta.free=0;
    block->meta.next=NULL;

    return block;
}

//-------------------malloc Implementation-------------------------

void *my_malloc(size_t size){
    if (size==0){
        return NULL;
    }
    size=ALIGN(size);

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
            while(last->meta.next){
                last=last->meta.next;
            }
            block=request_memory(size);
            if(!block)
                return NULL;

            last->meta.next=block;
        }else{
            block->meta.free=0;
        }
    }

    return (void *)(block+1);
}

//-------------------free Implementation------------------------- 

void my_free(void *ptr){
    if(!ptr)
        return;
    block_t *block=(block_t *)ptr-1;
    block->meta.free=1;
}
