#include "xddos/graphics.h"
#include "xddos/logging.h"
#include "xddos/memalloc.h"
#include "xddos/pit.h"
#include "xddos/pmm.h"
#include "xddos/psf.h"
#include "xddos/requests.h"
#include "xddos/serial.h"
#include "xddos/vma.h"
#include "xddos/vmm.h"
#include <stddef.h>
#include <stdint.h>

static void hcf() {
	for (;;) {
		__asm__("hlt");
	}
}

static void panic(xddos_framebuffer_t *fb) {
	(void) fb;
	LOG_ERROR("KERNEL", "Kernel panic! Something went wrong.");
	hcf();
}

void kernel_main() {
	if (xddos_request_base_revision_supported() == 0) {
		hcf();
	}

	xddos_framebuffers_t *fbs = xddos_request_framebuffers();
	if (fbs == NULL || fbs->count < 1) hcf();

	xddos_framebuffer_t *fb = fbs->framebuffers[0];

	xddos_serial_init();

	LOG_INFO("xD-DOS", "Extended Drive - Disk Operating System (xddos) Starting...");

	// PMM init
	LOG_INFO("KERNEL", "Physical Memory Manager initializing...");
	uint8_t pmm_result = xddos_pmm_init();
	if (pmm_result == 1) {
		LOG_ERROR("KERNEL", "Failed to initialize Physical Memory Manager! Error code 0x%x (Memory Map or HHDM Not Ready).", pmm_result);
		panic(fb);
		return;
	} else if (pmm_result == 2) {
		LOG_ERROR("KERNEL", "Failed to initialize Physical Memory Manager! Error code 0x%x (No Memory Available for Bitmap).", pmm_result);
		panic(fb);
		return;
	} else if (pmm_result > 0) {
		LOG_ERROR("KERNEL", "Failed to initialize Physical Memory Manager! Error code 0x%x (Unknown).", pmm_result);
		panic(fb);
		return;
	}
	LOG_INFO("KERNEL", "Physical Memory Manager initialized.");

	// VMM init
	LOG_INFO("KERNEL", "Virtual Memory Manager initializing...");
	uint8_t vmm_result = xddos_vmm_init();
	if (vmm_result == 1) {
		LOG_ERROR("KERNEL", "Failed to initialize Virtual Memory Manager! Error code 0x%x (HHDM or Executable Address or Executable File Not Ready).", vmm_result);
		panic(fb);
		return;
	} else if (vmm_result == 2) {
		LOG_ERROR("KERNEL", "Failed to initialize Virtual Memory Manager! Error code 0x%x (HHDM Offset is 0).", vmm_result);
		panic(fb);
		return;
	} else if (vmm_result == 3) {
		LOG_ERROR("KERNEL", "Failed to initialize Virtual Memory Manager! Error code 0x%x (Out of Memory).", vmm_result);
		panic(fb);
		return;
	} else if (vmm_result > 0) {
		LOG_ERROR("KERNEL", "Failed to initialize Virtual Memory Manager! Error code 0x%x (Unknown).", vmm_result);
		panic(fb);
		return;
	}
	LOG_INFO("KERNEL", "Virtual Memory Manager initialized.");

	// VMA init
	LOG_INFO("KERNEL", "Virtual Memory Allocator initializing...");
	xddos_vma_init(0xFFFFFFFF90000000ULL);
	LOG_INFO("KERNEL", "Virtual Memory Allocator initialized.");

	// Allocate initial heap
	LOG_INFO("KERNEL", "Initial heap allocating...");
	size_t initial_heap_bytes = 128 * 1024;
	size_t initial_pages = initial_heap_bytes / PAGE_SIZE;

	void *initial_heap_block = xddos_vma_alloc_pages(initial_pages);
	if (!initial_heap_block) {
		LOG_ERROR("KERNEL", "Failed to allocate initial heap pages!");
		panic(fb);
		return;
	}

	xddos_memalloc_init(xddos_vma_alloc_pages, initial_heap_block, initial_heap_bytes);
	LOG_INFO("KERNEL", "Initial heap allocated.");

	// Init font
	LOG_INFO("KERNEL", "Font initializing...");
	xddos_psf_data_t *font = xddos_psf_init();
	LOG_INFO("KERNEL", "Font initialized.");

	xddos_graphics_clear(fb, 0);
	const char *msg = "xD-DOS (Extended Drive - Disk Operating System)\r\nMaintained by Jason Christian\r\n\r\n";
	xddos_graphics_clear(fb, 0x000000);
	xddos_graphics_put_text(fb, font, msg, 4, 4, 0xFFFFFF, 0x000000);
	SLEEP(2000);

	// Halt
	LOG_INFO("KERNEL", "Nothing to do, halting...");
	hcf();
}