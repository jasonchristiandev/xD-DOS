#ifndef VMM_H
#define VMM_H

#include <stdint.h>

#define MEMORY_PTE_PRESENT (1ULL << 0)
#define MEMORY_PTE_WRITABLE (1ULL << 1)
#define MEMORY_PTE_USER (1ULL << 2)
#define MEMORY_PTE_HUGE (1ULL << 7)
#define MEMORY_PTE_FRAME 0x000FFFFFFFFFF000ULL
#define MEMORY_PHYS_TO_VIRT(phys) ((uint64_t)(phys) + (uint64_t)(hhdm_offset))
#define MEMORY_VIRT_TO_PHYS(virt) ((uint64_t)((uintptr_t)(virt)) - (uint64_t)(hhdm_offset))

typedef struct xddos_vmm_page_table {
	uint64_t entries[512];
} xddos_vmm_page_table_t;

uint8_t xddos_vmm_init();
uint8_t xddos_vmm_map_table(xddos_vmm_page_table_t *pml4, uint64_t virt, uint64_t phys, uint64_t flags);
uint8_t xddos_vmm_map_table_huge(xddos_vmm_page_table_t *pml4, uint64_t virt, uint64_t phys, uint64_t flags);

#endif // !VMM_H