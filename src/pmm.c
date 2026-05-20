#include "xD-DOS/pmm.h"
#include "xD-DOS/logging.h"
#include "xD-DOS/memory.h" // IWYU pragma: keep
#include "xD-DOS/requests.h"
#include <stddef.h>
#include <stdint.h>

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

static inline uint8_t bitmap_test(uint64_t bit) {
	return (bitmap[bit / 8] & (1 << (bit % 8))) != 0;
}

void pmm_free_region(uint64_t base_address, uint64_t length) {
	uint64_t start_page = (base_address + PAGE_SIZE - 1) / PAGE_SIZE;
	uint64_t end_page = (base_address + length) / PAGE_SIZE;

	for (uint64_t i = start_page; i < end_page; i++) {
		if (i < total_pages) {
			bitmap_clear(i);
		}
	}
}

void pmm_lock_region(uint64_t base_address, uint64_t length) {
	uint64_t start_page = base_address / PAGE_SIZE;
	uint64_t end_page = (base_address + length + PAGE_SIZE - 1) / PAGE_SIZE;

	for (uint64_t i = start_page; i < end_page; i++) {
		if (i < total_pages) {
			bitmap_set(i);
		}
	}
}

// Initializes the Physical Memory Manager
uint8_t pmm_init() {
	DEBUG_INFO("PMM", "Checking responses...");
	xD_DOS_memmap *memmap = request_memmap();
	xD_DOS_hhdm *hhdm = request_hhdm();

	if (memmap == NULL || hhdm == NULL) return 1;

	hhdm_offset = hhdm->offset;

	uint64_t highest_address = 0;
	xD_DOS_memmap_entry *best_chunk = NULL;

	DEBUG_INFO("PMM", "Searching usable physical memory...");

	for (uint64_t i = 0; i < memmap->count; i++) {
		xD_DOS_memmap_entry *entry = memmap->entries[i];

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

	if (!best_chunk) return 2;

	total_pages = highest_address / PAGE_SIZE;
	bitmap_size = total_pages / 8;
	if (total_pages % 8 != 0) bitmap_size++;

	if (best_chunk->length < bitmap_size) {
		return 2; // Not enough space in the best chunk
	}

	DEBUG_INFO("PMM", "Allocating...");

	// Virtual pointer so the CPU can write to the bitmap safely
	bitmap = (uint8_t *) (best_chunk->base + hhdm_offset);

	DEBUG_INFO("PMM", "Clearing... (Setting ALL memory to LOCKED)");
	// Lock every physical page by default.
	memset(bitmap, 0xFF, bitmap_size);

	DEBUG_INFO("PMM", "Parsing region for bitmap...");

	// Free only usable memory
	for (uint64_t i = 0; i < memmap->count; i++) {
		if (memmap->entries[i]->type == XD_DOS_MEMMAP_USABLE) {
			pmm_free_region(memmap->entries[i]->base, memmap->entries[i]->length);
		}
	}

	DEBUG_INFO("PMM", "Protecting bitmap...");
	// Lock the memory that the bitmap itself is occupying.
	pmm_lock_region(best_chunk->base, bitmap_size);

	pmm_lock_region(0, PAGE_SIZE);

	return 0;
}

// Allocates a single page
void *pmm_alloc_page() {
	size_t bitmap_bytes = total_pages / 8;

	for (size_t i = 0; i < bitmap_bytes; i++) {
		if (bitmap[i] == 0xFF) continue;

		// Found a byte with at least one free bit
		for (int bit = 0; bit < 8; bit++) {
			if ((bitmap[i] & (1 << bit)) == 0) {
				size_t page_index = (i * 8) + bit;

				if (page_index >= total_pages) return NULL;

				// Lock it
				bitmap[i] |= (1 << bit);

				// Return the raw PHYSICAL address
				return (void *) (page_index * PAGE_SIZE);
			}
		}
	}
	return NULL; // Out of physical memory
}

// Frees a single page
void pmm_free_page(void *page) {
	if (!page) return;

	uint64_t page_index = (uint64_t) page / PAGE_SIZE;

	if (page_index >= total_pages) {
		DEBUG_ERROR("PMM", "Attempted to free out of bounds page: %x", page);
		return;
	}

	bitmap[page_index / 8] &= ~(1 << (page_index % 8));
}