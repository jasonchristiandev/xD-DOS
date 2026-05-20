#include "xD-DOS/pmm.h"
#include "xD-DOS/logging.h"
#include "xD-DOS/memory.h" // IWYU pragma: keep
#include <limine.h>
#include <stddef.h>
#include <stdint.h>

extern volatile struct limine_memmap_request memmap_request;
extern volatile struct limine_hhdm_request hhdm_request;

static uint8_t *bitmap = NULL;
static size_t total_pages = 0;
static size_t bitmap_size = 0;
static uint64_t hhdm_offset = 0;

// Initializes the Physical Memory Manager
// Returns 1 if memory map or HHDM (Higher Half Direct Map) is not ready.
// Returns 2 if no memory is available for the bitmap.
// Returns 0 otherwise.
uint8_t pmm_init() {
	DEBUG_INFO("PMM", "Checking responses...");
	struct limine_memmap_response *memmap = memmap_request.response;
	struct limine_hhdm_response *hhdm = hhdm_request.response;

	if (!memmap || !hhdm) return 1;

	hhdm_offset = hhdm->offset;

	uint64_t highest_address = 0;
	struct limine_memmap_entry *best_chunk = NULL;

	// Find the top of usable physical memory so we know how big our bitmap must be
	DEBUG_INFO("PMM", "Searching usable physical memory...");
	DEBUG_INFO("PMM", "Memmap Address = %x, Entries Array = %x, Count = %d",
			   (void *) memmap, (void *) memmap->entries, (int) memmap->entry_count);

	if (memmap->entries == NULL && memmap->entry_count > 0) {
		DEBUG_ERROR("PMM", "Memmap entries array pointer is NULL despite count > 0!");
		return 1;
	}

	for (uint64_t i = 0; i < memmap->entry_count; i++) {
		struct limine_memmap_entry *entry = memmap->entries[i];

		if (entry->type == LIMINE_MEMMAP_USABLE) {
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

	// Place the bitmap at the start of our best usable memory chunk
	bitmap = (uint8_t *) (best_chunk->base + hhdm_offset);

	DEBUG_INFO("PMM", "Clearing...");
	memset(bitmap, 0xFF, bitmap_size);

	uint64_t bitmap_pages = (bitmap_size + PAGE_SIZE - 1) / PAGE_SIZE;
	uint64_t bitmap_phys_start = best_chunk->base;
	uint64_t bitmap_phys_end = bitmap_phys_start + (bitmap_pages * PAGE_SIZE);

	// Mark only the usable regions from memmap as free
	DEBUG_INFO("PMM", "Parsing region for bitmap...");
	for (uint64_t i = 0; i < memmap->entry_count; i++) {
		struct limine_memmap_entry *entry = memmap->entries[i];

		if (entry->type == LIMINE_MEMMAP_USABLE) {
			uint64_t start_page = entry->base / PAGE_SIZE;
			uint64_t page_count = entry->length / PAGE_SIZE;

			for (uint64_t j = 0; j < page_count; j++) {
				uint64_t page = start_page + j;
				uint64_t page_phys_addr = page * PAGE_SIZE;

				if (page_phys_addr >= bitmap_phys_start && page_phys_addr < bitmap_phys_end) {
					continue;
				}

				bitmap[page / 8] &= ~(1 << (page % 8));
			}
		}
	}

	// Protect the memory that the bitmap itself is occupying
	DEBUG_INFO("PMM", "Protecting bitmap...");
	uint64_t bitmap_physical_base = best_chunk->base;

	for (uint64_t i = 0; i < bitmap_pages; i++) {
		uint64_t page = (bitmap_physical_base / PAGE_SIZE) + i;
		bitmap[page / 8] |= (1 << (page % 8));
	}

	return 0;
}

// Allocates a single page
// Returns NULL if out of physical memory
void *pmm_alloc_page() {
	size_t bitmap_bytes = total_pages / 8;

	for (size_t i = 0; i < bitmap_bytes; i++) {
		if (bitmap[i] == 0xFF) continue;

		// Found a byte with at least one free bit
		for (int bit = 0; bit < 8; bit++) {
			if ((bitmap[i] & (1 << bit)) == 0) {
				size_t page_index = (i * 8) + bit;

				if (page_index >= total_pages) return NULL;

				bitmap[i] |= (1 << bit);
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