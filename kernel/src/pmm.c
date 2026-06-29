#include "xddos/pmm.h"
#include "xddos/logging.h"
#include "xddos/requests.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static uint8_t *bitmap = NULL;
static size_t bitmap_size = 0;
static size_t page_count = 0;
static uint64_t hhdm_offset = 0;

xddos_pmm_init_result_t xddos_pmm_init() {
	// get hhdm offset
	xddos_memmap_t *memmap = xddos_request_memmap();
	if (memmap == NULL) {
		LOG_ERROR("PMM", "Memory map request responded with NULL!");
		return XDDOS_PMM_INIT_NULL_RESPONSE;
	}
	xddos_hhdm_t *hhdm = xddos_request_hhdm();
	if (hhdm == NULL) {
		LOG_ERROR("PMM", "HHDM request responded with NULL!");
		return XDDOS_PMM_INIT_NULL_RESPONSE;
	}
	hhdm_offset = hhdm->offset;

	// find best spot for bitmap
	xddos_memmap_entry_t *max = NULL;
	uint64_t max_addr = 0;
	LOG_DEBUG("PMM", "Finding spot for bitmap...");
	for (uint64_t i = 0; i < memmap->count; i++) {
		xddos_memmap_entry_t *entry = memmap->entries[i];
		if (entry->type != XD_DOS_MEMMAP_USABLE) continue;

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
		return XDDOS_PMM_INIT_OUT_OF_SPACE;
	}
	LOG_DEBUG("PMM", "Found memory chunk for bitmap at 0x%llx.", max);

	page_count = max_addr / PAGE_SIZE;
	bitmap_size = page_count / 8; // one byte maps 8 pages
	if (page_count % 8 != 0) bitmap_size++;

	if (max->length < bitmap_size) {
		LOG_ERROR("PMM", "Not enough space to fit bitmap! (0x%llx)", max);
		return XDDOS_PMM_INIT_OUT_OF_SPACE;
	}

	bitmap = (uint8_t *) (max->base + hhdm_offset);
	memset(bitmap, 0, bitmap_size);

	// lock regions
	LOG_DEBUG("PMM", "Locking bitmap... (addr: 0x%llx, size: %d)", max->base, bitmap_size);
	xddos_pmm_lock_region(max->base, bitmap_size); // bitmap
	LOG_DEBUG("PMM", "Locking page 0...");
	xddos_pmm_lock_region(0, PAGE_SIZE);

	LOG_DEBUG("PMM", "Done init.");

	return XDDOS_PMM_INIT_OK;
}

void xddos_pmm_lock_region(uint64_t base, uint64_t count) {
	uint64_t start = base / PAGE_SIZE;
	uint64_t end = (base + count + PAGE_SIZE - 1) / PAGE_SIZE;

	for (uint64_t i = start; i < end; i++) {
		if (i < page_count) {
			bitmap[i / 8] |= (1 << (i % 8));
		}
	}
}

void xddos_pmm_free_region(uint64_t base, uint64_t count) {
	uint64_t start = base / PAGE_SIZE;
	uint64_t end = (base + count + PAGE_SIZE - 1) / PAGE_SIZE;

	for (uint64_t i = start; i < end; i++) {
		if (i < page_count) {
			bitmap[i / 8] &= ~(1 << (i % 8));
		}
	}
}

uint8_t *xddos_pmm_alloc_page() {
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

void xddos_pmm_free_page(uint8_t *page) {
	if (page == NULL) return;

	uint64_t idx = (uint64_t) page / PAGE_SIZE;

	if (idx >= page_count) {
		LOG_ERROR("PMM", "Attempted to free out of bound page!", page);
		return;
	}

	bitmap[idx / 8] &= ~(1 << (idx % 8));
}
