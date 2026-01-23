#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "allocator.h"

int main(void)
{
    printf("Test 1: integer allocation\n");

    int *a = my_malloc(sizeof(int) * 5);
    assert(a != NULL);

    for (int i = 0; i < 5; i++)
        a[i] = i * 10;

    for (int i = 0; i < 5; i++)
        printf("%d ", a[i]);
    printf("\n");

    my_free(a);

    printf("Test 2: string allocation\n");

    char *msg = my_malloc(32);
    assert(msg != NULL);

    strcpy(msg, "Hello custom allocator");
    printf("%s\n", msg);

    my_free(msg);

    printf("Test 3: alignment check\n");

    void *p = my_malloc(sizeof(long double));
    assert(p != NULL);
    assert(((uintptr_t)p % ALLOCATOR_ALIGNMENT) == 0);

    my_free(p);

    printf("Test 4: block splitting & reuse\n");

    void *d = my_malloc(100);
    void *e = my_malloc(100);
    void *f = my_malloc(100);

    assert(d && e && f);

    my_free(e);

    void *g = my_malloc(80);   // should reuse freed block `e`
    assert(g == e);            // VERY IMPORTANT CHECK

    my_free(d);
    my_free(f);
    my_free(g);


     
    printf("Test 5: backward + forward coalescing\n");

    void *x = my_malloc(128);
    void *y = my_malloc(256);
    void *z = my_malloc(128);

    assert(x && y && z);

    /*
        Heap layout:
        [ x ][ y ][ z ]
    */

    my_free(y);   // free middle
    my_free(x);   // backward coalesce should merge x + y
    my_free(z);   // forward coalesce completes merge

    /*
        Expected:
        [ one large free block ]
    */

    void *big = my_malloc(128 + 256 + 128);
    assert(big == x);   // MUST reuse from start

    my_free(big);

    printf("Backward coalescing test passed\n");

    printf("ALL basic tests passed\n");
    return 0;
}

