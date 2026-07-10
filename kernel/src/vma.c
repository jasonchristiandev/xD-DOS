#include "xddos/vma.h"
#include "xddos/logging.h"
#include "xddos/pmm.h"
#include "xddos/vmm.h"

extern xddos_vmm_page_table_t *xddos_pml4;

static uint64_t heap_current_break = 0;
static uint64_t heap_start = 0;

void xddos_vma_init(uint64_t heap_base) {
	heap_start = heap_base;
	heap_current_break = heap_base;
	LOG_DEBUG("VMA", "Done init.");
}

// Allocates contiguous virtual pages and maps them to physical frames
void *xddos_vma_alloc_pages(size_t pages) {
	if (pages == 0) return NULL;

	uint64_t virt_start = heap_current_break;

	// Allocate physical frames and map each page individually
	for (size_t i = 0; i < pages; i++) {
		uint64_t virt_addr = virt_start + (i * PAGE_SIZE);
		void *phys_frame = xddos_pmm_alloc_page();

		if (phys_frame == NULL) return NULL;

		// Map physical frame
		xddos_vmm_map_table(xddos_pml4, virt_addr, (uint64_t) phys_frame, XDDOS_PTE_READWRITE);
	}

	// Advance the break pointer
	heap_current_break += (pages * PAGE_SIZE);

	return (void *) virt_start;
}