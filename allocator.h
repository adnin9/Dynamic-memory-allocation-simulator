#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stdbool.h>
#include <stddef.h>

#define MEMORY_SIZE 1024
#define MAX_BLOCKS 100

typedef struct {
    int start;
    int size;
    bool is_free;
} MemoryBlock;

void* my_malloc(int size);
void my_free(void* ptr);
void* my_realloc(void* ptr, int new_size);
void defragment_memory();
void handle_leaks();
void cleanup_memory();
void reset();
void print_memory_layout();

#endif