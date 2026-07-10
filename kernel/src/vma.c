#include "xddos/vma.h"
#include "xddos/logging.h"
#include "xddos/pmm.h"
#include "xddos/vmm.h"
#include <stdbool.h>

const size_t HEAP_SIZE = 1024 * 1024 * 16; // 16mb

struct Block {
	size_t size;
	bool is_free;
	struct Block *next;
};

void xddos_vma_init(uint64_t base) {
	for (size_t i = 0; i < HEAP_SIZE / PAGE_SIZE; i++) {
		void *page = xddos_pmm_alloc_page();
		if (!page) {
			LOG_ERROR("VMA", "Out of memory during heap init!");
			return;
		}
		uint64_t addr = base + i * PAGE_SIZE;
		xddos_vmm_map_table(addr, (uint64_t) page, XDDOS_PTE_READWRITE);
	}
	LOG_DEBUG("VMA", "Done init.");
}

void *xddos_vma_malloc(size_t size) {
}

void xddos_vma_free(void *ptr) {
}

void *xddos_vma_calloc(size_t count, size_t size) {
}