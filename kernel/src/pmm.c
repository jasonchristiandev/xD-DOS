#include "xddos/pmm.h"
#include "xddos/logging.h"
#include "xddos/requests.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static uint8_t *bitmap = NULL;
static size_t total_pages = 0;
static size_t bitmap_size = 0;
static uint64_t hhdm_offset = 0;

static inline void bitmap_set(uint64_t bit) {
	bitmap[bit / 8] |= (1 << (bit % 8));
}

static inline void bitmap_clear(uint64_t bit) {
	bitmap[bit / 8] &= ~(1 << (bit % 8));
}

void xddos_pmm_free_region(uint64_t base_address, uint64_t length) {
	uint64_t start_page = (base_address + PAGE_SIZE - 1) / PAGE_SIZE;
	uint64_t end_page = (base_address + length) / PAGE_SIZE;

	for (uint64_t i = start_page; i < end_page; i++) {
		if (i < total_pages) {
			bitmap_clear(i);
		}
	}
}

void xddos_pmm_lock_region(uint64_t base_address, uint64_t length) {
	uint64_t start_page = base_address / PAGE_SIZE;
	uint64_t end_page = (base_address + length + PAGE_SIZE - 1) / PAGE_SIZE;

	for (uint64_t i = start_page; i < end_page; i++) {
		if (i < total_pages) {
			bitmap_set(i);
		}
	}
}

xddos_pmm_init_result_t xddos_pmm_init() {
	LOG_DEBUG("PMM", "Checking responses...");
	xddos_memmap_t *memmap = xddos_request_memmap();
	xddos_hhdm_t *hhdm = xddos_request_hhdm();

	if (memmap == NULL || hhdm == NULL) {
		LOG_ERROR("PMM", "Memmap or HHDM is NULL!");
		return XDDOS_PMM_NO_RESPONSES;
	}

	hhdm_offset = hhdm->offset;

	uint64_t highest_address = 0;
	xddos_memmap_entry_t *best_chunk = NULL;

	LOG_DEBUG("PMM", "Searching usable physical memory...");

	for (uint64_t i = 0; i < memmap->count; i++) {
		xddos_memmap_entry_t *entry = memmap->entries[i];

		if (entry->type == XD_DOS_MEMMAP_USABLE) {
			uint64_t top = entry->base + entry->length;
			if (top > highest_address) {
				highest_address = top;
			}
			if (!best_chunk || entry->length > best_chunk->length) {
				best_chunk = entry;
			}
		}
	}

	if (!best_chunk) {
		LOG_ERROR("PMM", "No usable memory to use for bitmap!");
		return XDDOS_PMM_OUT_OF_SPACE;
	}

	LOG_DEBUG("PMM", "Found memory chunk to put bitmap (0x%llx)", best_chunk);

	total_pages = highest_address / PAGE_SIZE;
	bitmap_size = total_pages / 8;
	if (total_pages % 8 != 0) bitmap_size++;

	if (best_chunk->length < bitmap_size) {
		LOG_ERROR("PMM", "Not enough space to fit bitmap! (0x%llx)", best_chunk);
		return 2;
	}

	bitmap = (uint8_t *) (best_chunk->base + hhdm_offset);

	LOG_DEBUG("PMM", "Clearing bitmap... (addr: 0x%llx, size: %d)", bitmap, bitmap_size);
	memset(bitmap, 0xFF, bitmap_size);

	LOG_DEBUG("PMM", "Parsing region for bitmap...");

	// Free only usable memory
	for (uint64_t i = 0; i < memmap->count; i++) {
		if (memmap->entries[i]->type == XD_DOS_MEMMAP_USABLE) {
			xddos_pmm_free_region(memmap->entries[i]->base, memmap->entries[i]->length);
		}
	}

	LOG_DEBUG("PMM", "Protecting bitmap... (addr: 0x%llx, size: %d)", best_chunk->base, bitmap_size);
	xddos_pmm_lock_region(best_chunk->base, bitmap_size); // bitmap
	LOG_DEBUG("PMM", "Protecting zero page... (addr: 0x%llx, size: %d)", 0, PAGE_SIZE);
	xddos_pmm_lock_region(0, PAGE_SIZE);

	LOG_DEBUG("PMM", "Done init.");

	return XDDOS_PMM_OK;
}

// Allocates a single page
void *xddos_pmm_alloc_page() {
	for (size_t i = 0; i < bitmap_size; i++) {
		if (bitmap[i] == 0xFF) continue;

		// Found a byte with at least one free bit
		for (uint8_t bit = 0; bit < 8; bit++) {
			if ((bitmap[i] & (1 << bit)) == 0) {
				size_t page_index = (i * 8) + bit;
				if (page_index >= total_pages) return NULL;
				bitmap[i] |= (1 << bit);

				return (void *) (page_index * PAGE_SIZE);
			}
		}
	}
	LOG_ERROR("PMM", "Out of physical memory!");
	return NULL; // Out of physical memory
}

// Frees a single page
void xddos_pmm_free_page(void *page) {
	if (!page) return;

	uint64_t page_index = (uint64_t) page / PAGE_SIZE;

	if (page_index >= total_pages) {
		LOG_ERROR("PMM", "Attempted to free out of bounds page: 0x%llx", page);
		return;
	}

	bitmap[page_index / 8] &= ~(1 << (page_index % 8));
}