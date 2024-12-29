//Question 1: first fit

#include <stdio.h>
#include <unistd.h>
#include "memorymanage.h"

int main() {
    printf("Heap Start Address: %p\n", sbrk(0));

    //allocate 
    void *ptrs[10];
    for (int i = 0; i < 10; i++) {
        ptrs[i] = my_malloc((i + 1) * 100);
        printf("Allocated %d bytes at %p\n", (i + 1) * 100, ptrs[i]);
    }

    //free
    for (int i = 0; i < 5; i++) {
        my_free(ptrs[i]);
        printf("Freeing memory at %p\n", ptrs[i]);
    }
    printf("Heap End Address: %p\n", sbrk(0));
    printf("Memory Leaks Detected: %zu bytes\n", detect_memory_leak());

    return 0;
}
