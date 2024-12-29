//Question 2: best fit

#ifndef MEMORYMANAGE_H
#define MEMORYMANAGE_H

#include <stddef.h>

void *my_malloc(size_t size);
void my_free(void *ptr);
void *my_calloc(size_t num, size_t size);
void *my_realloc(void *ptr, size_t new_size);
size_t calculate_memory_leaks(void);
size_t detect_memory_leak();

#endif
