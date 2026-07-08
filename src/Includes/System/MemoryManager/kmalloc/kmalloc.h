#pragma once
#include <stddef.h>

void kmalloc_init();

void kfree(void* ptr);
void* kmalloc(size_t size);

typedef struct {
    size_t used_bytes;
    size_t free_bytes;
} tlsf_stats_t;
tlsf_stats_t MemHeapInfo();

void test_kernel_heap();
