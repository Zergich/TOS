#pragma once

#include <types.h>

#define PAGE_SIZE 4096

void pmm_init();
uptr pmm_alloc_frame();
void pmm_free_frame(uptr addr);
u64 pmm_get_free_pages();

void test_pmm();
