#ifndef PMM_H
#define PMM_H

#include <stdint.h>

#define PAGE_SIZE 4096

typedef enum : uint8_t {
	PMM_INIT_OK = 0,
	PMM_INIT_NULL_RESPONSE = 1,
	PMM_INIT_OUT_OF_SPACE = 2
} pmm_init_result_t;

typedef struct {
	uint64_t base;
	uint64_t length;
	uint64_t type;
} pmm_memmap_entry_t;
typedef struct {
	uint64_t count;
	pmm_memmap_entry_t **entries;
} pmm_memmap_t;

pmm_init_result_t pmm_init();
void pmm_free_region(uint64_t base, uint64_t count);
void pmm_lock_region(uint64_t base, uint64_t count);
uint8_t *pmm_alloc_page();
void pmm_free_page(uint8_t *page);

#endif // !PMM_H