#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>
#include <stdalign.h>

#define ALLOCATOR_ALIGNMENT alignof(max_align_t)
/* Public API */
void *my_malloc(size_t size);
void  my_free(void *ptr);
void *my_realloc(void *ptr, size_t size);

#endif /* ALLOCATOR_H */

