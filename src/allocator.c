#include "allocator.h"
#include <unistd.h>
#include <stddef.h>
#include <stdalign.h>

/* =======================
   Block Metadata
   ======================= */

typedef struct block {
    struct {
        size_t size;          // payload size
        int free;             // 1 = free, 0 = used
        struct block *next;   // next block in heap
    } meta;

    max_align_t _align;       // ensures payload alignment
} block_t;

/* =======================
   Globals & Macros
   ======================= */

static block_t *head_heap = NULL;

#define ALIGNMENT ALLOCATOR_ALIGNMENT
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

#define BLOCK_SIZE  ALIGN(sizeof(block_t))
#define FOOTER_SIZE ALIGN(sizeof(size_t))

/* =======================
   Helper Functions
   ======================= */

static void write_footer(block_t *block) {
    size_t *footer = (size_t *)(
        (char *)block + BLOCK_SIZE + block->meta.size
    );
    *footer = block->meta.size;
}

/* -----------------------
   Find free block (first-fit)
   ----------------------- */
static block_t *find_free_block(size_t size) {
    block_t *current = head_heap;

    while (current) {
        if (current->meta.free && current->meta.size >= size)
            return current;
        current = current->meta.next;
    }
    return NULL;
}

/* -----------------------
   Request memory from OS
   ----------------------- */
static block_t *request_memory(size_t size) {
    void *request = sbrk(0);
    void *allocate = sbrk(BLOCK_SIZE + size + FOOTER_SIZE);

    if (allocate == (void *)-1)
        return NULL;

    block_t *block = (block_t *)request;
    block->meta.size = size;
    block->meta.free = 0;
    block->meta.next = NULL;

    write_footer(block);
    return block;
}

/* -----------------------
   Split block
   ----------------------- */
static void split_block(block_t *block, size_t size) {
    size = ALIGN(size);

    if (block->meta.size >= size + BLOCK_SIZE + FOOTER_SIZE + ALIGNMENT) {
        block_t *new_block = (block_t *)(
            (char *)block + BLOCK_SIZE + size + FOOTER_SIZE
        );

        new_block->meta.size = block->meta.size - size - BLOCK_SIZE - FOOTER_SIZE;
        new_block->meta.free = 1;
        new_block->meta.next = block->meta.next;

        block->meta.size = size;
        block->meta.next = new_block;

        write_footer(block);
        write_footer(new_block);
    }
}

/* -----------------------
   Forward coalescing
   ----------------------- */
static void coalesce_forward(block_t *block) {
    while (block->meta.next && block->meta.next->meta.free) {
        block_t *next = block->meta.next;

        block->meta.size += BLOCK_SIZE + FOOTER_SIZE + next->meta.size;
        block->meta.next = next->meta.next;
    }
    write_footer(block);
}

/* -----------------------
   Get previous block using footer
   ----------------------- */
static block_t *get_prev_block(block_t *block) {
    if (block == head_heap)
        return NULL;

    size_t *prev_footer = (size_t *)(
        (char *)block - FOOTER_SIZE
    );

    size_t prev_size = *prev_footer;

    block_t *prev = (block_t *)(
        (char *)block - FOOTER_SIZE - prev_size - BLOCK_SIZE
    );

    return prev;
}

/* -----------------------
   Backward coalescing
   ----------------------- */
static block_t *coalesce_backward(block_t *block) {
    block_t *prev = get_prev_block(block);

    if (prev && prev->meta.free) {
        prev->meta.size += BLOCK_SIZE + FOOTER_SIZE + block->meta.size;
        prev->meta.next = block->meta.next;

        write_footer(prev);
        return prev;
    }
    return block;
}

/* =======================
   malloc
   ======================= */

void *my_malloc(size_t size) {
    if (size == 0)
        return NULL;

    size = ALIGN(size);

    block_t *block;

    if (!head_heap) {
        block = request_memory(size);
        if (!block)
            return NULL;
        head_heap = block;
    } else {
        block = find_free_block(size);
        if (!block) {
            block_t *last = head_heap;
            while (last->meta.next)
                last = last->meta.next;

            block = request_memory(size);
            if (!block)
                return NULL;

            last->meta.next = block;
        } else {
            split_block(block, size);
            block->meta.free = 0;
        }
    }

    return (void *)(block + 1);
}

/* =======================
   free
   ======================= */

void my_free(void *ptr) {
    if (!ptr)
        return;

    block_t *block = (block_t *)ptr - 1;
    block->meta.free = 1;

    block = coalesce_backward(block);
    coalesce_forward(block);
}

