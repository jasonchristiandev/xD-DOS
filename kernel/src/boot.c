#include "xddos/graphics.h"
#include "xddos/interrupts.h"
#include "xddos/logging.h"
#include "xddos/pmm.h"
#include "xddos/requests.h"
#include "xddos/serial.h"
#include "xddos/vmm.h"
#include <stddef.h>

#ifndef COMMIT_HASH
#define COMMIT_HASH "unknown"
#endif // !COMMIT_HASH

void boot_main(boot_info_t *info) {
	boot_info = info;

	// init serial
	serial_init();

	LOG_INFO("KERNEL", "xD-DOS (%s) Starting...", COMMIT_HASH);
	LOG_DEBUG("KERNEL", "BOOT INFO:");
	LOG_DEBUG("KERNEL", "    HHDM: 0x%llx", boot_info->hhdm);
	LOG_DEBUG("KERNEL", "    Memmap (0x%llx):", boot_info->memmap);
	LOG_DEBUG("KERNEL", "        Count: %d", boot_info->memmap->count);
	LOG_DEBUG("KERNEL", "        Entries (0x%llx)", boot_info->memmap->entries);
	LOG_DEBUG("KERNEL", "    Executable File Info (0x%llx):", boot_info->exefile);
	LOG_DEBUG("KERNEL", "        Address: 0x%llx", boot_info->exefile->address);
	LOG_DEBUG("KERNEL", "        Size: 0x%x", boot_info->exefile->size);
	LOG_DEBUG("KERNEL", "    Executable Address Info (0x%llx):", boot_info->exeaddr);
	LOG_DEBUG("KERNEL", "        Physical: 0x%llx", boot_info->exeaddr->phys);
	LOG_DEBUG("KERNEL", "        Virtual: 0x%llx", boot_info->exeaddr->virt);
	LOG_DEBUG("KERNEL", "    Framebuffers (0x%llx):", boot_info->framebuffers);
	LOG_DEBUG("KERNEL", "        Count: %d", boot_info->framebuffers->count);
	LOG_DEBUG("KERNEL", "        Entries (0x%llx):", boot_info->framebuffers->entries);
	for (uint64_t i = 0; i < boot_info->framebuffers->count; i++) {
		LOG_DEBUG("KERNEL", "            Entry %d (0x%llx):", i, boot_info->framebuffers->entries[i]);
		LOG_DEBUG("KERNEL", "                Address: 0x%llx", boot_info->framebuffers->entries[i]->address);
		LOG_DEBUG("KERNEL", "                Bit/Pixel: 0x%x", boot_info->framebuffers->entries[i]->bpp);
		LOG_DEBUG("KERNEL", "                Size: 0x%x", boot_info->framebuffers->entries[i]->size);
		LOG_DEBUG("KERNEL", "                Pitch: 0x%x", boot_info->framebuffers->entries[i]->pitch);
		LOG_DEBUG("KERNEL", "                Resolution: %dx%d", boot_info->framebuffers->entries[i]->width, boot_info->framebuffers->entries[i]->height);
	}

	// framebuffer
	requests_framebuffers_t *fbs = request_framebuffers();
	if (fbs == NULL || fbs->count < 1) {
		interrupts_fail("No framebuffer!", 1, "KERNEL");
	}
	fb = fbs->entries[0];

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