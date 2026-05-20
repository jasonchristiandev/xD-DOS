#include "xD-DOS/font.h"
#include "xD-DOS/logging.h"
#include "xD-DOS/memalloc.h"
#include "xD-DOS/pmm.h"
#include "xD-DOS/requests.h"
#include "xD-DOS/termgraphics.h" // IWYU pragma: keep
#include "xD-DOS/serial.h"
#include "xD-DOS/vma.h"
#include "xD-DOS/vmm.h"
#include <stddef.h>
#include <stdint.h>

#define PANIC() \
	panic(fb);  \
	return;

static void hcf() {
	for (;;) {
		__asm__("hlt");
	}
}

static void panic(xD_DOS_framebuffer *fb) {
	LOG_ERROR("KERNEL", "Kernel panic! Something went wrong.");
	hcf();
}

void kernel_main() {
	if (request_base_revision_supported() == 0) {
		hcf();
	}

	xD_DOS_framebuffers *fbs = request_framebuffers();
	if (fbs == NULL || fbs->count < 1) hcf();

	xD_DOS_framebuffer *fb = fbs->framebuffers[0];
	volatile uint32_t *fb_ptr = fb->address;

	serial_init();

	LOG_INFO("xD-DOS", "Extended Drive - Disk Operating System (xD-DOS) Starting...");

	// PMM init
	LOG_INFO("KERNEL", "Physical Memory Manager initializing...");
	uint8_t pmm_result = pmm_init();
	if (pmm_result == 1) {
		LOG_ERROR("KERNEL", "Failed to initialize Physical Memory Manager! Error code 0x%x (Memory Map or HHDM Not Ready).", pmm_result);
		PANIC();
	} else if (pmm_result == 2) {
		LOG_ERROR("KERNEL", "Failed to initialize Physical Memory Manager! Error code 0x%x (No Memory Available for Bitmap).", pmm_result);
		PANIC();
	} else if (pmm_result > 0) {
		LOG_ERROR("KERNEL", "Failed to initialize Physical Memory Manager! Error code 0x%x (Unknown).", pmm_result);
		PANIC();
	}
	DELETE_PREV_LINE();
	LOG_INFO("KERNEL", "Physical Memory Manager initialized.");

	// VMM init
	LOG_INFO("KERNEL", "Virtual Memory Manager initializing...");
	uint8_t vmm_result = vmm_init();
	DELETE_PREV_LINE();
	LOG_INFO("KERNEL", "Virtual Memory Manager initialized.");

	// VMA init
	LOG_INFO("KERNEL", "Virtual Memory Allocator initializing...");
	vma_init(0xFFFFFFFF90000000ULL);
	DELETE_PREV_LINE();
	LOG_INFO("KERNEL", "Virtual Memory Allocator initialized.");

	// Allocate initial heap
	LOG_INFO("KERNEL", "Initial heap allocating...");
	size_t initial_heap_bytes = 128 * 1024;
	size_t initial_pages = initial_heap_bytes / PAGE_SIZE;

	void *initial_heap_block = vma_alloc_pages(initial_pages);
	if (!initial_heap_block) {
		LOG_ERROR("KERNEL", "Failed to allocate initial heap pages!");
		PANIC();
	}

	malloc_init(vma_alloc_pages, initial_heap_block, initial_heap_bytes);
	DELETE_PREV_LINE();
	LOG_INFO("KERNEL", "Initial heap allocated.");

	// Init font
	LOG_INFO("KERNEL", "Font initializing...");
	psf_init();
	DELETE_PREV_LINE();
	LOG_INFO("KERNEL", "Font initialized.");

	putchar(fb, 'c', 1, 1, 0xFFFFFF, 0xFFFFFF);

	// Halt
	LOG_INFO("KERNEL", "Nothing to do, halting...");
	PANIC(); // panic testing
}