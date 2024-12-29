//Question 1: first fit

#include <unistd.h> 
#include <string.h>  
#include "memorymanage.h"

typedef struct Block {
    size_t size;
    int free;
    struct Block *next; 
} Block;

#define BLOCK_SIZE sizeof(Block)

static Block *head = NULL; 
static Block *tail = NULL;  

Block *find_free_block(size_t size) {
    Block *current = head;
    while (current) {
        if (current->free && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

Block *extend_heap(size_t size) {
    Block *block = sbrk(0);
    void *request = sbrk(size + BLOCK_SIZE);
    if (request == (void *)-1) {
        return NULL; 
    }
    block->size = size;
    block->free = 0;
    block->next = NULL;

    if (tail) {
        tail->next = block;
    }
    tail = block;

    if (!head) {
        head = block;
    }
    return block;
}

void split_block(Block *block, size_t size) {
    Block *new_block = (void *)((char *)block + BLOCK_SIZE + size);
    new_block->size = block->size - size - BLOCK_SIZE;
    new_block->free = 1;
    new_block->next = block->next;
    block->size = size;
    block->next = new_block;
}

//malloc 
void *my_malloc(size_t size) {
    if (size <= 0) {
        return NULL;
    }

    Block *block = find_free_block(size);
    if (block) {
        if (block->size > size + BLOCK_SIZE) {
            split_block(block, size);
        }
        block->free = 0;
    } else {
        block = extend_heap(size);
        if (!block) {
            return NULL;
        }
    }
    return (char *)block + BLOCK_SIZE;
}

void merge_blocks() {
    Block *current = head;
    while (current && current->next) {
        if (current->free && current->next->free) {
            current->size += current->next->size + BLOCK_SIZE;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

//free
void my_free(void *ptr) {
    if (!ptr) {
        return;
    }

    Block *block = (Block *)((char *)ptr - BLOCK_SIZE);
    block->free = 1;

    merge_blocks();
}

//calloc 
void *my_calloc(size_t num, size_t size) {
    size_t total_size = num * size;
    void *ptr = my_malloc(total_size);
    if (ptr) {
        memset(ptr, 0, total_size);
    }
    return ptr;
}

//realloc 
void *my_realloc(void *ptr, size_t new_size) {
    if (!ptr) {
        return my_malloc(new_size); 
    }

    Block *block = (Block *)((char *)ptr - BLOCK_SIZE);
    if (block->size >= new_size) {
        return ptr; 
    }

    void *new_ptr = my_malloc(new_size);
    if (new_ptr) {
        memcpy(new_ptr, ptr, block->size);
        my_free(ptr);
    }
    return new_ptr;
}

size_t calculate_memory_leaks() {
    size_t leaks = 0;
    Block *current = head;
    while (current) {
        if (!current->free) {
            leaks += current->size;
        }
        current = current->next;
    }
    return leaks;
}

size_t detect_memory_leak(void) {
    return calculate_memory_leaks();
}

