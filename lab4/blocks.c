#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/mman.h>
#include <string.h>

#define MAX_BLOCK_SIZE (1 << 20)
#define MIN_BLOCK_SIZE 8
#define MAP_ANONYMOUS 0x20

typedef struct Allocator {
    void *memory;
    size_t size;
    void **free_blocks;
    size_t num_levels;
} Allocator;

size_t round_up_pow2(size_t size) {
    size_t power = MIN_BLOCK_SIZE;
    while (power < size) {
        power *= 2;
    }
    return power;
}

Allocator* allocator_create(void *const memory, const size_t size) {
    if (size < MIN_BLOCK_SIZE) {
        fprintf(stderr, "Memory size too small\n");
        return NULL;
    }
    Allocator *allocator = mmap(NULL, sizeof(Allocator), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (allocator == MAP_FAILED) {
        perror("mmap");
        return NULL;
    }
    allocator->memory = memory;
    allocator->size = size;
    allocator->num_levels = log2(size / MIN_BLOCK_SIZE) + 1;
    allocator->free_blocks = mmap(NULL, allocator->num_levels * sizeof(void*), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (allocator->free_blocks == MAP_FAILED) {
        perror("mmap");
        munmap(allocator, sizeof(Allocator));
        return NULL;
    }
    memset(allocator->free_blocks, 0, allocator->num_levels * sizeof(void*));
    allocator->free_blocks[allocator->num_levels - 1] = memory;
    return allocator;
}

void* allocator_alloc(Allocator *const allocator, const size_t size) {
    if (!allocator || size > allocator->size) {
        return NULL;
    }
    size_t block_size = round_up_pow2(size);
    size_t level = log2(block_size / MIN_BLOCK_SIZE);
    for (size_t i = level; i < allocator->num_levels; i++) {
        if (allocator->free_blocks[i] != NULL) {
            void *block = allocator->free_blocks[i];
            allocator->free_blocks[i] = *(void**)block;
            while (i > level) {
                i--;
                void *buddy = (void*)((char*)block + (1 << (MIN_BLOCK_SIZE + i)));
                *(void**)buddy = allocator->free_blocks[i];
                allocator->free_blocks[i] = buddy;
            }

            return block;
        }
    }
    return NULL;
}

void allocator_free(Allocator *const allocator, void *const memory) {
    if (!allocator || !memory) {
        return;
    }
    size_t offset = (char*)memory - (char*)allocator->memory;
    if (offset >= allocator->size) {
        return;
    }
    size_t level = 0;
    size_t block_size = MIN_BLOCK_SIZE;
    while (block_size < allocator->size && (offset % (block_size * 2)) == 0) {
        block_size *= 2;
        level++;
    }
    void *buddy = (void*)((char*)allocator->memory + (offset ^ block_size));
    void **current = &allocator->free_blocks[level];
    while (*current) {
        if (*current == buddy) {
            *current = *(void**)(*current);
            allocator_free(allocator, (offset < (char*)buddy - (char*)allocator->memory) ? memory : buddy);
            return;
        }
        current = (void**)*current;
    }
    *(void**)memory = allocator->free_blocks[level];
    allocator->free_blocks[level] = memory;
}

void allocator_destroy(Allocator *const allocator) {
    if (!allocator) {
        return;
    }
    munmap(allocator->free_blocks, allocator->num_levels * sizeof(void*));
    munmap(allocator, sizeof(Allocator));
}
