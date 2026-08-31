#include "xddos/pmm.h"
#include "xddos/logging.h"
#include "xddos/requests.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static uint8_t *bitmap = NULL;
static size_t bitmap_size = 0;
static size_t page_count = 0;

pmm_init_result_t pmm_init() {
	requests_memmap_t *memmap = boot_info.memmap;
	if (memmap == NULL) {
		LOG_ERROR("PMM", "Memory map request responded with NULL!");
		return PMM_INIT_NULL_RESPONSE;
	}

	// find best spot for bitmap
	requests_memmap_entry_t *max = NULL;
	uint64_t max_addr = 0;
	LOG_DEBUG("PMM", "Finding spot for bitmap...");
	for (uint64_t i = 0; i < memmap->count; i++) {
		requests_memmap_entry_t *entry = &(memmap->entries[i]);
		if (entry->type != REQUESTS_MEMMAP_USABLE || entry->length == 0 || entry->base < 0x100000 || entry->base + entry->length > 0x40000000) continue;
		LOG_DEBUG("PMM", "Entry %u: 0x%llx - 0x%llx (%llu KB)", i, entry->base, entry->base + entry->length - 1, entry->length / 1024);

		uint64_t top = entry->base + entry->length;
		if (top > max_addr) {
			max_addr = top;
		}
		if (!max || entry->length > max->length) {
			max = entry;
		}
	}

	if (max == NULL) {
		LOG_ERROR("PMM", "No memory chunk available for bitmap!");
		return PMM_INIT_OUT_OF_SPACE;
	}
	LOG_DEBUG("PMM", "Found memory chunk for bitmap at 0x%llx.", max->base);

	page_count = max_addr / PAGE_SIZE;
	bitmap_size = page_count / 8; // one byte maps 8 pages
	if (page_count % 8 != 0) bitmap_size++;

	if (max->length < bitmap_size) {
		LOG_ERROR("PMM", "Not enough space to fit bitmap! (0x%llx, required %llu got %llu)", max, bitmap_size, max->length);
		return PMM_INIT_OUT_OF_SPACE;
	}

	bitmap = (uint8_t *) max->base;
	memset(bitmap, 0xFF, bitmap_size);

	// free regions
	LOG_DEBUG("PMM", "Freeing usable regions...");
	for (uint64_t i = 0; i < memmap->count; i++) {
		requests_memmap_entry_t entry = memmap->entries[i];
		if (entry.type == REQUESTS_MEMMAP_USABLE && entry.base > 0x100000) {
			pmm_free_region(entry.base, entry.length);
		}
	}

	pmm_lock_region((uint64_t) NULL, PAGE_SIZE);
	pmm_lock_region((uint64_t) bitmap, bitmap_size);

	LOG_DEBUG("PMM", "Done init.");

	return PMM_INIT_OK;
}

void pmm_lock_region(uint64_t base, uint64_t count) {
	uint64_t start = base / PAGE_SIZE;
	uint64_t end = (base + count + PAGE_SIZE - 1) / PAGE_SIZE;

	for (uint64_t i = start; i < end; i++) {
		if (i < page_count) {
			bitmap[i / 8] |= (1 << (i % 8));
		}
	}
}

void pmm_free_region(uint64_t base, uint64_t count) {
	uint64_t start = base / PAGE_SIZE;
	uint64_t end = (base + count + PAGE_SIZE - 1) / PAGE_SIZE;

	for (uint64_t i = start; i < end; i++) {
		if (i < page_count) {
			bitmap[i / 8] &= ~(1 << (i % 8));
		}
	}
}

uint8_t *pmm_alloc_page() {
	for (uint64_t i = 0; i < bitmap_size; i++) {
		if (bitmap[i] == 0b11111111) continue; // if byte is full then skip

		for (uint8_t bit = 0; bit < 8; bit++) {
			if ((bitmap[i] & (1 << bit)) != 0) continue; // if bit is full then skip

			uint64_t idx = i * 8 + bit;
			if (idx >= page_count) return NULL; // tried to use memory out of bounds aka out of memory
			bitmap[i] |= (1 << bit);

			return (uint8_t *) (idx * PAGE_SIZE);
		}
	}

	return NULL; // no memory available aka out of memory
}

void pmm_free_page(uint8_t *page) {
	if (page == NULL) return;
	uint64_t idx = (uint64_t) page / PAGE_SIZE;
	if (idx >= page_count) return;
	bitmap[idx / 8] &= ~(1 << (idx % 8));
}
