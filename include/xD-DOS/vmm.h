#ifndef VMM_H
#define VMM_H

#include <stddef.h>
#include <stdint.h>

#define PTE_PRESENT (1ULL << 0)
#define PTE_WRITABLE (1ULL << 1)
#define PTE_USER (1ULL << 2)
#define PTE_FRAME 0x000FFFFFFFFFF000ULL
#define phys_to_virt(phys) ((uint64_t)(phys))
#define virt_to_phys(virt) ((uint64_t)(virt))

typedef struct {
	uint64_t entries[512];
} page_table_t;

uint8_t vmm_map_table(page_table_t *pml4, uint64_t virt, uint64_t phys, uint64_t flags);

#endif // !VMM_H