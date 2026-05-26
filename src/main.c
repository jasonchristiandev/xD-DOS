#include "xddos/psf.h"
#include "xddos/graphics.h"
#include "xddos/logging.h"
#include "xddos/stdlib.h" // IWYU pragma: keep
#include "xddos/pit.h"
#include "xddos/pmm.h"
#include "xddos/requests.h"
#include "xddos/serial.h"
#include "xddos/string.h" // IWYU pragma: keep
#include "xddos/vma.h"
#include "xddos/vmm.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static void hcf() {
	for (;;) {
		__asm__("hlt");
	}
}

static void panic(xddos_framebuffer_t *fb) {
	LOG_ERROR("KERNEL", "Kernel panic! Something went wrong.");
	hcf();
}

void kernel_main() {
	if (request_base_revision_supported() == 0) {
		hcf();
	}

	xddos_framebuffers_t *fbs = request_framebuffers();
	if (fbs == NULL || fbs->count < 1) hcf();

	xddos_framebuffer_t *fb = fbs->framebuffers[0];
	volatile uint32_t *fb_ptr = fb->address;

	serial_init();

	LOG_INFO("xD-DOS", "Extended Drive - Disk Operating System (xddos) Starting...");

	// PMM init
	LOG_INFO("KERNEL", "Physical Memory Manager initializing...");
	uint8_t pmm_result = pmm_init();
	if (pmm_result == 1) {
		LOG_ERROR("KERNEL", "Failed to initialize Physical Memory Manager! Error code 0x%llx (Memory Map or HHDM Not Ready).", pmm_result);
		panic(fb);
		return;
	} else if (pmm_result == 2) {
		LOG_ERROR("KERNEL", "Failed to initialize Physical Memory Manager! Error code 0x%llx (No Memory Available for Bitmap).", pmm_result);
		panic(fb);
		return;
	} else if (pmm_result > 0) {
		LOG_ERROR("KERNEL", "Failed to initialize Physical Memory Manager! Error code 0x%llx (Unknown).", pmm_result);
		panic(fb);
		return;
	}
	LOG_INFO("KERNEL", "Physical Memory Manager initialized.");

	// VMM init
	LOG_INFO("KERNEL", "Virtual Memory Manager initializing...");
	uint8_t vmm_result = vmm_init();
	LOG_INFO("KERNEL", "Virtual Memory Manager initialized.");

	// VMA init
	LOG_INFO("KERNEL", "Virtual Memory Allocator initializing...");
	vma_init(0xFFFFFFFF90000000ULL);
	LOG_INFO("KERNEL", "Virtual Memory Allocator initialized.");

	// Allocate initial heap
	LOG_INFO("KERNEL", "Initial heap allocating...");
	size_t initial_heap_bytes = 128 * 1024;
	size_t initial_pages = initial_heap_bytes / PAGE_SIZE;

	void *initial_heap_block = vma_alloc_pages(initial_pages);
	if (!initial_heap_block) {
		LOG_ERROR("KERNEL", "Failed to allocate initial heap pages!");
		panic(fb);
		return;
	}

	memalloc_init(vma_alloc_pages, initial_heap_block, initial_heap_bytes);
	LOG_INFO("KERNEL", "Initial heap allocated.");

	// Init font
	LOG_INFO("KERNEL", "Font initializing...");
	xddos_psf_data_t *font = psf_init();
	LOG_INFO("KERNEL", "Font initialized.");

	xddos_graphics_clear(fb, 0);
	const char *msg = "xD-DOS (Extended Drive - Disk Operating System)\r\nMaintained by Jason Christian\r\n\r\n";
	xddos_graphics_put_text(fb, font, msg, 4, 4, 0xFFFFFF, 0x000000);
	SLEEP(2000);

	// Halt
	LOG_INFO("KERNEL", "Nothing to do, halting...");
	hcf();
}