#include "xddos/vma.h"
#include "xddos/logging.h"
#include "xddos/pmm.h"
#include "xddos/vmm.h"
#include <stdbool.h>
#include <string.h>

typedef struct heap_block {
	size_t size;
	bool free;
	struct heap_block *next;
} heap_block_t;

heap_block_t *heap_head;
heap_block_t *heap_end;

vma_init_result_t vma_init() {
	for (size_t i = 0; i < HEAP_SIZE / PAGE_SIZE; i++) {
		void *page = pmm_alloc_page();
		if (!page) return VMA_INIT_OUT_OF_MEMORY;
		uint64_t addr = HEAP_BASE + i * PAGE_SIZE;
		vmm_map_table(addr, (uint64_t) page, PTE_READWRITE);
	}

	heap_head = (heap_block_t *) HEAP_BASE;
	heap_head->size = HEAP_SIZE - sizeof(heap_block_t);
	heap_head->free = true;
	heap_head->next = NULL;
	heap_end = (heap_block_t *) (HEAP_BASE + HEAP_SIZE);

	LOG_DEBUG("VMA", "Done init.");
	return VMA_INIT_OK;
}

void *vma_malloc(size_t size) {
	if (size == 0) return NULL;

	size = (size + 7) & ~7;

	heap_block_t *cur = heap_head;

	while (cur != NULL) {
		if (cur->free && cur->size >= size) {
			if (cur->size > size + sizeof(heap_block_t) + 8) {
				heap_block_t *block = (heap_block_t *) ((uint8_t *) cur + sizeof(heap_block_t) + size);
				block->size = cur->size - size - sizeof(heap_block_t);
				block->free = true;
				block->next = cur->next;

				cur->size = size;
				cur->next = block;
			}

			cur->free = false;
			return (void *) ((uint8_t *) cur + sizeof(heap_block_t));
		}

		if (cur->next == NULL) {
			size_t required = size + sizeof(heap_block_t);
			size_t pages = (required + PAGE_SIZE - 1) / PAGE_SIZE;

			for (size_t i = 0; i < pages; i++) {
				void *page = pmm_alloc_page();
				if (!page) return NULL;
				uint64_t addr = (uint64_t) heap_end + i * PAGE_SIZE;
				vmm_map_table(addr, (uint64_t) page, PTE_READWRITE);
			}

			heap_block_t *block = (heap_block_t *) heap_end;
			block->size = pages * PAGE_SIZE - sizeof(heap_block_t);
			block->free = true;
			block->next = NULL;

			heap_end = (heap_block_t *) ((uint8_t *) heap_end + pages * PAGE_SIZE);

			if (cur->free) {
				cur->size += sizeof(heap_block_t) + block->size;
			} else {
				cur->next = block;
				cur = block;
			}
		} else {
			cur = cur->next;
		}
	}

	return NULL;
}

void vma_free(void *ptr) {
	if (!ptr) return;

	heap_block_t *block = (heap_block_t *) ((uint8_t *) ptr - sizeof(heap_block_t));
	block->free = true;

	heap_block_t *cur = heap_head;
	while (cur != NULL && cur->next != NULL) {
		if (cur->free && cur->next->free) {
			cur->size += sizeof(heap_block_t) + cur->next->size;
			cur->next = cur->next->next;
		} else {
			cur = cur->next;
		}
	}
}

void *vma_calloc(size_t count, size_t size) {
	void *ptr = vma_malloc(count * size);
	memset(ptr, 0, count * size);
	return ptr;
}