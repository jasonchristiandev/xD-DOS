#ifndef PMM_H
#define PMM_H

#include <stdint.h>

#define PAGE_SIZE 4096

uint8_t xddos_pmm_init();
void xddos_pmm_free_region(uint64_t base_address, uint64_t length);
void xddos_pmm_lock_region(uint64_t base_address, uint64_t length);
void* xddos_pmm_alloc_page();
void xddos_pmm_free_page(void *page);

#endif // !PMM_H