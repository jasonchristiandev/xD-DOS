#include "xD-DOS/vma.h"
#include "xD-DOS/logging.h"
#include "xD-DOS/pmm.h"
#include "xD-DOS/vmm.h"

extern page_table_t *kernel_pml4;

static uint64_t heap_current_break = 0;
static uint64_t heap_start = 0;

void vma_init(uint64_t heap_base) {
	heap_start = heap_base;
	heap_current_break = heap_base;
	DEBUG_INFO("VMA", "Virtual Heap initialized at: %llx", (unsigned long long) heap_start);
}

// Allocates contiguous virtual pages and maps them to physical frames
void *vma_alloc_pages(size_t pages) {
	if (pages == 0) return NULL;

	uint64_t alloc_virt_start = heap_current_break;

	// Allocate physical frames and map each page individually
	for (size_t i = 0; i < pages; i++) {
		uint64_t virt_addr = alloc_virt_start + (i * PAGE_SIZE);
		void *phys_frame = pmm_alloc_page();

		if (phys_frame == NULL) {
			DEBUG_ERROR("VMA", "Out of physical memory during virtual allocation!");
			// Realistically, you'd want an unmap loop here to clean up partial allocations.
			return NULL;
		}

		// Map physical frame
		vmm_map_table(kernel_pml4, virt_addr, (uint64_t) phys_frame, PTE_WRITABLE);
	}

	// Advance the break pointer
	heap_current_break += (pages * PAGE_SIZE);

	return (void *) alloc_virt_start;
}