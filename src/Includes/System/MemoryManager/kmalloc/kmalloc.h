#pragma once
#include <stddef.h>

void kmalloc_init();

void kfree(void* ptr);
void* kmalloc(size_t size);



void test_kernel_heap();
