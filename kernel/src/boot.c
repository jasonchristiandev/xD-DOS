#include "xddos/asm.h"
#include "xddos/graphics.h"
#include "xddos/interrupts.h"
#include "xddos/logging.h"
#include "xddos/multiboot2.h"
#include "xddos/pmm.h"
#include "xddos/requests.h"
#include "xddos/serial.h"
#include "xddos/vmm.h"
#include <stddef.h>

#ifndef COMMIT_HASH
#define COMMIT_HASH "unknown"
#endif // !COMMIT_HASH

void boot_main(uint32_t magic, uint8_t *mb_tags) {
	// init serial
	serial_init();

	if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
		LOG_ERROR("KERNEL", "Multiboot2 bootloader magic invalid! (0x%x)", magic);
		hlt();
	}

	// boot info
	uint32_t mb_tags_size = *((uint32_t *) mb_tags);

	uint8_t *curr = (uint8_t *) mb_tags + 8;
	uint8_t *end = (uint8_t *) mb_tags + mb_tags_size;
	bool ended = false;

	while (curr < end) {
		multiboot_tag_t *base_tag = (multiboot_tag_t *) curr;

		if (base_tag->size < 8) {
			LOG_ERROR("KERNEL", "Multiboot2 boot information corrupt!\r\nCorrupt tag with invalid size %u!", base_tag->size);
			break;
		}

		if (base_tag->type == 0) {
			ended = true;
			break;
		}

		switch (base_tag->type) {
			case MULTIBOOT_TAG_TYPE_MMAP:
				static boot_memmap_t memmap;
				boot_info.memmap = &memmap;

				multiboot_tag_mmap_t *tag = (multiboot_tag_mmap_t *) base_tag;
				uint32_t count = (tag->size - 16) / sizeof(multiboot_mmap_entry_t);
				multiboot_mmap_entry_t *entries = tag->entries;

				uint64_t memmap_idx = 0;

				for (uint32_t i = 0; i < count; i++) {
					if (memmap_idx >= MAX_MEMMAP_ENTRIES) break;
					multiboot_mmap_entry_t *entry = (multiboot_mmap_entry_t *) entries;

					if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
						boot_memmap_entry_t *mmentry = &memmap.entries[memmap_idx];
						mmentry->type = BOOT_MEMMAP_USABLE;
						mmentry->base = entry->addr;
						mmentry->length = entry->len;
						if (entry->addr < 0x1000) { // avoid null (0x0-0x1000)
							memmap_idx--;
						}
						memmap_idx++;
					}

					entries = (multiboot_mmap_entry_t *) ((uint8_t *) entries + tag->entry_size);
				}
				memmap.count = memmap_idx;
				if (memmap.count > MAX_MEMMAP_ENTRIES) memmap.count = MAX_MEMMAP_ENTRIES;

				break;
			case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
				break;
			case MULTIBOOT_TAG_TYPE_SMBIOS:
				break;
			case MULTIBOOT_TAG_TYPE_ACPI_NEW:
				break;
			case MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR:
				break;
			default:
				LOG_WARNING("KERNEL", "Unknown/unimplemented Multiboot2 tag %u", base_tag->type);
				break;
		}

		curr += (base_tag->size + 7) & ~7;
	}
	if (!ended) {
		LOG_ERROR("KERNEL", "Multiboot2 boot information corrupt!\r\nBoot information not ended with end tag.");
		hlt();
	}

	// framebuffer
	requests_framebuffers_t *fbs = request_framebuffers();
	if (fbs != NULL && fbs->count >= 1) fb = fbs->entries[0];

	// init fallback font
	LOG_DEBUG("KERNEL", "Initializing fallback font...");
	fallback_font = psf_init();

	// init pmm
	LOG_DEBUG("KERNEL", "Physical Memory Manager initializing...");
	pmm_init_result_t pmm_result = pmm_init();
	char *pmm_result_name[3] = {
		[PMM_INIT_OK] = "OK",
		[PMM_INIT_NULL_RESPONSE] = "NULL_RESPONSE",
		[PMM_INIT_OUT_OF_SPACE] = "OUT_OF_SPACE"};
	if (pmm_result != 0) {
		char *name = "UNKNOWN";
		if (pmm_result < 3) name = pmm_result_name[pmm_result];
		interrupts_fail("PMM_INIT bad return!", pmm_result, name);
	}

	// init vmm
	LOG_DEBUG("KERNEL", "Virtual Memory Manager initializing...");
	vmm_init_result_t vmm_result = vmm_init();
	char *vmm_result_name[3] = {
		[VMM_INIT_OK] = "OK",
		[VMM_INIT_NULL_RESPONSE] = "NULL_RESPONSE",
		[VMM_INIT_OUT_OF_MEMORY] = "OUT_OF_SPACE"};
	if (vmm_result != 0) {
		char *name = "UNKNOWN";
		if (vmm_result < 3) name = vmm_result_name[vmm_result];
		interrupts_fail("VMM_INIT bad return!", vmm_result, name);
	}

	// jump to kernel_main (hopefully)
}