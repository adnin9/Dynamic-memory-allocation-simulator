#include <stdio.h>
#include <string.h>
#include "allocator.h"

unsigned char memory[MEMORY_SIZE];
MemoryBlock blocks[MAX_BLOCKS];
int block_count = 0;

// defragmentation: merge adjacent free blocks
void defragment_memory() {
    for (int i = 0; i < block_count - 1; ) {
        if (blocks[i].is_free && blocks[i + 1].is_free) {
            blocks[i].size += blocks[i + 1].size;

            // shift remaining blocks left
            for (int j = i + 1; j < block_count - 1; j++) {
                blocks[j] = blocks[j + 1];
            }
            block_count--;
        } else {
            i++;
        }
    }
}

// custom malloc
void* my_malloc(int size) {
    if (size > MEMORY_SIZE) {
        return NULL;
    }
    defragment_memory();

    for (int i = 0; i < block_count; i++) {
        if (blocks[i].is_free && blocks[i].size >= size) {
            blocks[i].is_free = false;

            if (blocks[i].size > size) {
                // split block if required size is less than free block
                for (int j = block_count; j > i + 1; j--) {
                    blocks[j] = blocks[j - 1];
                }
                blocks[i + 1].start = blocks[i].start + size;
                blocks[i + 1].size = blocks[i].size - size;
                blocks[i + 1].is_free = true;
                blocks[i].size = size;
                block_count++;
            }

            return &memory[blocks[i].start];
        }
    }

    // try to append new block at end
    int last_end = 0;
    if (block_count > 0) {
        MemoryBlock last = blocks[block_count - 1];
        last_end = last.start + last.size;
    }

    if (last_end + size > MEMORY_SIZE) {
        printf("Error: Not enough memory :o\n");
        return NULL;
    }

    blocks[block_count].start = last_end;
    blocks[block_count].size = size;
    blocks[block_count].is_free = false;
    block_count++;

    return &memory[last_end];
}

// custom free
void my_free(void* ptr) {
    int offset = (unsigned char*)ptr - memory;

    for (int i = 0; i < block_count; i++) {
        if (blocks[i].start == offset && !blocks[i].is_free) {
            blocks[i].is_free = true;
            printf("Freed memory at offset %d\n\n", offset);
            defragment_memory();
            return;
        }
    }

    printf("Error: Invalid pointer to free.\n");
}

void* my_realloc(void* ptr, int new_size) {
    if (ptr == NULL) {
        return my_malloc(new_size); // behaves like malloc
    }

    if (new_size == 0) {
        my_free(ptr); // behaves like free
        return NULL;
    }

    int offset = (unsigned char*)ptr - memory;

    for (int i = 0; i < block_count; i++) {
        if (blocks[i].start == offset && !blocks[i].is_free) {
            int old_size = blocks[i].size;

            // case 1: new size is smaller or equal
            if (new_size <= old_size) {
                if (new_size < old_size) {
                    // shrink and create a free block if possible
                    if (block_count < MAX_BLOCKS) {
                        for (int j = block_count; j > i + 1; j--) {
                            blocks[j] = blocks[j - 1];
                        }
                        blocks[i + 1].start = blocks[i].start + new_size;
                        blocks[i + 1].size = old_size - new_size;
                        blocks[i + 1].is_free = true;
                        blocks[i].size = new_size;
                        block_count++;
                    }
                }
                defragment_memory();
                return ptr;
            }

            // case 2: try to expand into next free block
            if (i + 1 < block_count &&
                blocks[i + 1].is_free &&
                blocks[i + 1].start == blocks[i].start + blocks[i].size) {

                int combined_size = blocks[i].size + blocks[i + 1].size;

                if (combined_size >= new_size) {
                    blocks[i].size = new_size;

                    if (combined_size > new_size) {
                        // resize the next block
                        blocks[i + 1].start = blocks[i].start + new_size;
                        blocks[i + 1].size = combined_size - new_size;
                    } else {
                        // remove the next block if fully consumed
                        for (int j = i + 1; j < block_count - 1; j++) {
                            blocks[j] = blocks[j + 1];
                        }
                        block_count--;
                    }
                    defragment_memory();

                    return ptr;
                }
            }

            // case 3: cannot expand — allocate new block
            void* new_ptr = my_malloc(new_size);
            if (new_ptr == NULL) {
                return NULL;
            }

            memcpy(new_ptr, ptr, old_size);
            my_free(ptr);
            return new_ptr;
        }
    }

    // invalid pointer
    return NULL;
}


void print_memory_layout() {
    if (block_count == 0) {
        printf("No memory allocated.\n\n");
        return;
    }

    printf("Memory Layout:\n");
    for (int i = 0; i < block_count; i++) {
        printf("Block %d: Start = %d, Size = %d, %s\n",
               i, blocks[i].start, blocks[i].size,
               blocks[i].is_free ? "FREE" : "USED");
    }
    printf("\n");
}

void reset() {
    memset(blocks, 0, sizeof(blocks));
    memset(memory, 0, sizeof(memory));
    block_count = 1;               // start with 1 big free block
    blocks[0].start = 0;
    blocks[0].size = MEMORY_SIZE;
    blocks[0].is_free = true;
}

void handle_leaks() {
    int leaks = 0;
    printf("Checking for memory leaks..\n");
    for (int i = 0; i < block_count; i++) {
        if (!blocks[i].is_free) {
            printf("Memory leak: Block at offset %d, size %d bytes not freed.\n",
                   blocks[i].start, blocks[i].size);
            leaks++;
            blocks[i].is_free = true;
            printf("Cleaning up leaked memory at offset %d, size %d bytes\n",
                   blocks[i].start, blocks[i].size);
        }
    }

    if (leaks == 0) {
        printf("No memory leaks detected.\n");
    }

    defragment_memory();

}