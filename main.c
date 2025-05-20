#include <stdio.h>
#include "allocator.h"

int main(void)
{
    reset();
    print_memory_layout();

    void *ptrA = my_malloc(256);
    void *ptrB = my_malloc(128);
    void *ptrC = my_malloc(300);

    print_memory_layout();

    my_free(ptrA);
    print_memory_layout();

    ptrB = my_realloc(ptrB, 200);
    printf("ptrB grown to 200 B\n\n");
    print_memory_layout();

    handle_leaks();
    print_memory_layout();

    return 0;
}