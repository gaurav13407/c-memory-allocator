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

     
    printf("Test 6: realloc behavior\n");

    /* -------- realloc(NULL, size) -------- */
    int *r1 = my_realloc(NULL, sizeof(int) * 4);
    assert(r1 != NULL);
    for (int i = 0; i < 4; i++)
        r1[i] = i + 1;

    /* -------- shrink realloc -------- */
    int *r2 = my_realloc(r1, sizeof(int) * 2);
    assert(r2 == r1);                 // must not move
    assert(r2[0] == 1 && r2[1] == 2); // data preserved

    /* -------- in-place grow realloc -------- */
    // layout trick: ensure next block is free
    void *pad = my_malloc(64);
    my_free(pad);

    int *r3 = my_realloc(r2, sizeof(int) * 6);
    assert(r3 == r2);                 // must grow in place
    assert(r3[0] == 1 && r3[1] == 2); // old data preserved

    for (int i = 2; i < 6; i++)
        r3[i] = i + 1;

    /* -------- fallback realloc (forced move) -------- */
    void *blocker = my_malloc(1024);  // block in-place expansion

    int *r4 = my_realloc(r3, sizeof(int) * 20);
    assert(r4 != NULL);

    for (int i = 0; i < 6; i++)
        assert(r4[i] == i + 1);       // data preserved after move

    my_free(blocker);
    my_free(r4);

    printf("realloc tests passed\n");

    printf("ALL basic tests passed\n");
    return 0;
}

