#ifndef PMM_H
#define PMM_H

#include <stddef.h>
#include <stdint.h>

#define PAGE_SIZE 4096

uint8_t pmm_init();
void pmm_free_region(uint64_t base_address, uint64_t length);
void pmm_lock_region(uint64_t base_address, uint64_t length);
void* pmm_alloc_page();
void pmm_free_page(void *page);

#endif // !PMM_H