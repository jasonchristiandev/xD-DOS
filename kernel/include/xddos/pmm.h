#ifndef PMM_H
#define PMM_H

#include <stdint.h>

#define PAGE_SIZE 4096

typedef enum xddos_pmm_init_result {
	XDDOS_PMM_OK = 0,
	XDDOS_PMM_NO_RESPONSES = 1,
	XDDOS_PMM_OUT_OF_SPACE = 2
} xddos_pmm_init_result_t;

xddos_pmm_init_result_t xddos_pmm_init();
void xddos_pmm_free_region(uint64_t base_address, uint64_t length);
void xddos_pmm_lock_region(uint64_t base_address, uint64_t length);
void* xddos_pmm_alloc_page();
void xddos_pmm_free_page(void *page);

#endif // !PMM_H