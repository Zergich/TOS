#pragma once

#include <types.h>

#define PAGE_SIZE 4096

void pmm_init();
uptr pmm_alloc_page();
void pmm_free_page(uptr addr);
u64 pmm_get_free_pages();
