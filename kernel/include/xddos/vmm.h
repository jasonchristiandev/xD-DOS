#ifndef VMM_H
#define VMM_H

#include <stdint.h>

#define STACK_BASE 0xFFFFFC0000000000

typedef enum : uint16_t {
	PTE_PRESENT = 1 << 0,
	PTE_READWRITE = 1 << 1,
	PTE_USER = 1 << 2,
	PTE_WRITETHROUGH = 1 << 3,
	PTE_CACHEDISABLE = 1 << 4,
	PTE_ACCESSED = 1 << 5,
	PTE_DIRTY = 1 << 6,
	PTE_PAGESIZE = 1 << 7,
	PTE_GLOBAL = 1 << 8,
	// PTE_ = 1 << 9,
	// PTE_ = 1 << 10,
	// PTE_ = 1 << 11
} vmm_pte_flag_t;

typedef enum : uint8_t {
	VMM_INIT_OK = 0,
	VMM_INIT_NULL_RESPONSE = 1,
	VMM_INIT_OUT_OF_MEMORY = 2,
} vmm_init_result_t;

vmm_init_result_t vmm_init();
void vmm_map_table(uint64_t virt, uint64_t phys, uint64_t flags);
void vmm_map_table_huge(uint64_t virt, uint64_t phys, uint64_t flags);

#endif // !VMM_H