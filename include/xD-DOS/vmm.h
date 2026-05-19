#ifndef VMM_H
#define VMM_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
	uint64_t entries[512];
} page_table_t;

uint8_t vmm_map_table(page_table_t *pml4, size_t virt, size_t phys, uint64_t flags);

#endif // !VMM_H