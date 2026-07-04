#ifndef VMM_H
#define VMM_H

#include <stdint.h>

typedef struct {
	uint64_t entries[512];
} xddos_vmm_page_table_t;

typedef enum : uint8_t {
	XDDOS_VMM_INIT_OK = 0,
	XDDOS_VMM_INIT_NO_RESPONSES = 1,
	XDDOS_VMM_INIT_OFFSET_ZERO = 2,
	XDDOS_VMM_INIT_OUT_OF_MEMORY = 3,
} xddos_vmm_init_result_t;

xddos_vmm_init_result_t xddos_vmm_init();
void xddos_vmm_map_table(xddos_vmm_page_table_t *pml4, uint64_t virt, uint64_t phys, uint64_t flags);
void xddos_vmm_map_table_huge(xddos_vmm_page_table_t *pml4, uint64_t virt, uint64_t phys, uint64_t flags);

#endif // !VMM_H