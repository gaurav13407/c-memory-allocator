#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "allocator.h"

int main(){
    printf("Test 1:integer allocation\n");

    int *a=my_malloc(sizeof(int)*5);
    if(!a){
        printf("Allocation Failed\n");
        return 1;
    }

    for (int i=0;i<5;i++)
        a[i]=i*10;

    for (int i=0;i<5;i++)
        printf("%d",a[i]);
    printf("\n");

    my_free(a);
    printf("Test 2:string allocation\n");

    char *msg=my_malloc(32);
    strcpy(msg,"Hello custom allocator");
    printf("%s\n",msg);

    my_free(msg);

    void *p=my_malloc(sizeof(long double));
    printf("addr=%p,aligned=%ld\n",
            p,((uintptr_t)p%ALLOCATOR_ALIGNMENT));

    printf("ALL basic test passes\n");
    return 0;
}
