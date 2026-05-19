#include "xD-DOS/pmm.h"
#include "xD-DOS/memory.h" // IWYU pragma: keep
#include <limine.h>
#include <stddef.h>
#include <stdint.h>

static volatile struct limine_memmap_request memmap_request = {
	.id = LIMINE_MEMMAP_REQUEST_ID,
	.revision = 0};
static volatile struct limine_hhdm_request hhdm_request = {
	.id = LIMINE_HHDM_REQUEST_ID,
	.revision = 0};

static uint8_t *bitmap = NULL;
static size_t total_pages = 0;
static size_t bitmap_size = 0;
static uint64_t hhdm_offset = 0;

// Initializes the Physical Memory Manager
// Returns 1 if memory map or HHDM (Higher Half Direct Map) is not ready.
// Returns 2 if no memory is available for the bitmap.
uint8_t pmm_init() {
	struct limine_memmap_response *memmap = memmap_request.response;
	struct limine_hhdm_response *hhdm = hhdm_request.response;

	if (!memmap || !hhdm) return 1;

	hhdm_offset = hhdm->offset;

	uint64_t highest_address = 0;
	struct limine_memmap_entry *best_chunk = NULL;

	// Find the top of usable physical memory so we know how big our bitmap must be
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

	// Place the bitmap at the start of our best usable memory chunk
	bitmap = (uint8_t *) (best_chunk->base + hhdm_offset);

	memset(bitmap, 0xFF, bitmap_size);

	uint64_t bitmap_pages = (bitmap_size + PAGE_SIZE - 1) / PAGE_SIZE;
	uint64_t bitmap_phys_start = best_chunk->base;
	uint64_t bitmap_phys_end = bitmap_phys_start + (bitmap_pages * PAGE_SIZE);

	// Mark only the usable regions from memmap as free
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
	uint64_t bitmap_physical_base = best_chunk->base;
	if (bitmap_size % PAGE_SIZE != 0) bitmap_pages++;

	for (uint64_t i = 0; i < bitmap_pages; i++) {
		uint64_t page = (bitmap_physical_base / PAGE_SIZE) + i;
		bitmap[page / 8] |= (1 << (page % 8));
	}

	return 0;
}

// Allocates a single page
// Returns NULL if out of physical memory
void *pmm_alloc_page() {
	for (size_t i = 0; i < total_pages; i++) {
		if ((bitmap[i / 8] & (1 << (i % 8))) == 0) {
			bitmap[i / 8] |= (1 << (i % 8));

			uint64_t phys_addr = i * PAGE_SIZE;
			return (void *) (phys_addr + hhdm_offset);
		}
	}
	return NULL; // Out of physical memory
}

// Frees a single page
void pmm_free_page(void *page) {
	if (!page) return;

	uint64_t virt_addr = (uint64_t) page;
	uint64_t phys_addr = virt_addr - hhdm_offset;
	uint64_t page_index = phys_addr / PAGE_SIZE;

	bitmap[page_index / 8] &= ~(1 << (page_index % 8));
}