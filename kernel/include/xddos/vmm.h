#ifndef VMM_H
#define VMM_H

#include <stdint.h>

typedef enum : uint16_t {
	XDDOS_PTE_PRESENT = 1 << 0,
	XDDOS_PTE_READWRITE = 1 << 1,
	XDDOS_PTE_USER = 1 << 2,
	XDDOS_PTE_WRITETHROUGH = 1 << 3,
	XDDOS_PTE_CACHEDISABLE = 1 << 4,
	XDDOS_PTE_ACCESSED = 1 << 5,
	XDDOS_PTE_DIRTY = 1 << 6,
	XDDOS_PTE_PAGESIZE = 1 << 7,
	XDDOS_PTE_GLOBAL = 1 << 8,
	// XDDOS_PTE_ = 1 << 9,
	// XDDOS_PTE_ = 1 << 10,
	// XDDOS_PTE_ = 1 << 11
} xddos_vmm_pte_flag_t;

typedef struct {
	uint64_t entries[512];
} __attribute__((packed)) xddos_vmm_page_table_t;

typedef enum : uint8_t {
	XDDOS_VMM_INIT_OK = 0,
	XDDOS_VMM_INIT_NULL_RESPONSE = 1,
	XDDOS_VMM_INIT_OUT_OF_MEMORY = 2,
} xddos_vmm_init_result_t;

xddos_vmm_init_result_t xddos_vmm_init();
void xddos_vmm_map_table(xddos_vmm_page_table_t *pml4, uint64_t virt, uint64_t phys, uint64_t flags);
void xddos_vmm_map_table_huge(xddos_vmm_page_table_t *pml4, uint64_t virt, uint64_t phys, uint64_t flags);

static xddos_vmm_page_table_t *xddos_pml4;

#endif // !VMM_H