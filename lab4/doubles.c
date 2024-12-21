#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/mman.h>
#include <string.h>

#define MIN_BLOCK_SIZE 8
#define MAX_BLOCK_SIZE (1 << 20)
#define MAP_ANONYMOUS 0x20

typedef struct Allocator {
    void *memory;
    size_t size;
    void **free_lists;
    size_t num_levels;
} Allocator;

size_t round_up_pow2(size_t size) {
    size_t power = MIN_BLOCK_SIZE;
    while (power < size) {
        power *= 2;
    }
    return power;
}

size_t get_level(size_t block_size) {
    return log2(block_size / MIN_BLOCK_SIZE);
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
    allocator->free_lists = mmap(NULL, allocator->num_levels * sizeof(void*), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (allocator->free_lists == MAP_FAILED) {
        perror("mmap");
        munmap(allocator, sizeof(Allocator));
        return NULL;
    }
    memset(allocator->free_lists, 0, allocator->num_levels * sizeof(void*));
    allocator->free_lists[allocator->num_levels - 1] = memory;
    return allocator;
}

void* allocator_alloc(Allocator *const allocator, const size_t size) {
    if (!allocator || size > allocator->size) {
        return NULL;
    }
    size_t block_size = round_up_pow2(size);
    size_t level = get_level(block_size);
    for (size_t i = level; i < allocator->num_levels; i++) {
        if (allocator->free_lists[i] != NULL) {
            void *block = allocator->free_lists[i];
            allocator->free_lists[i] = *(void**)block;
            while (i > level) {
                i--;
                void *buddy = (void*)((char*)block + (1 << (MIN_BLOCK_SIZE + i)));
                *(void**)buddy = allocator->free_lists[i];
                allocator->free_lists[i] = buddy;
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
    void **current = &allocator->free_lists[level];
    while (*current) {
        if (*current == buddy) {
            *current = *(void**)(*current);
            allocator_free(allocator, (offset < (char*)buddy - (char*)allocator->memory) ? memory : buddy);
            return;
        }
        current = (void**)*current;
    }
    *(void**)memory = allocator->free_lists[level];
    allocator->free_lists[level] = memory;
}

void allocator_destroy(Allocator *const allocator) {
    if (!allocator) {
        return;
    }
    munmap(allocator->free_lists, allocator->num_levels * sizeof(void*));
    munmap(allocator, sizeof(Allocator));
}