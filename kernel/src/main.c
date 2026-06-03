#include "xddos/asm.h"
#include "xddos/graphics.h"
#include "xddos/interrupts.h"
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
	xddos_psf_data_t *fallback_font;

	xddos_serial_init();

	LOG_INFO("KERNEL", "Extended Drive - Disk Operating System (xD-DOS) Starting...");

	// Interrupts
	LOG_INFO("KERNEL", "Initializing IDT (Interrupt Descriptor Table)...");
	xddos_interrupts_init();

	LOG_INFO("KERNEL", "Initializing memory...");
	// PMM init
	LOG_DEBUG("KERNEL", "Physical Memory Manager initializing...");
	xddos_pmm_init_result_t pmm_result = xddos_pmm_init();
	if (pmm_result == XDDOS_PMM_NO_RESPONSES) {
		LOG_ERROR("KERNEL", "Failed to initialize Physical Memory Manager! Error code 0x%x (Memory Map or HHDM Not Ready).", pmm_result);
		panic(fb);
		return;
	} else if (pmm_result == XDDOS_PMM_OUT_OF_SPACE) {
		LOG_ERROR("KERNEL", "Failed to initialize Physical Memory Manager! Error code 0x%x (No Memory Available for Bitmap).", pmm_result);
		panic(fb);
		return;
	} else if (pmm_result > 0) {
		LOG_ERROR("KERNEL", "Failed to initialize Physical Memory Manager! Error code 0x%x (Unknown).", pmm_result);
		panic(fb);
		return;
	}

	// VMM init
	LOG_DEBUG("KERNEL", "Virtual Memory Manager initializing...");
	xddos_vmm_init_result_t vmm_result = xddos_vmm_init();
	if (vmm_result == XDDOS_VMM_NO_RESPONSES) {
		LOG_ERROR("KERNEL", "Failed to initialize Virtual Memory Manager! Error code 0x%x (HHDM or Executable Address or Executable File Not Ready).", vmm_result);
		panic(fb);
		return;
	} else if (vmm_result == XDDOS_VMM_OFFSET_ZERO) {
		LOG_ERROR("KERNEL", "Failed to initialize Virtual Memory Manager! Error code 0x%x (HHDM Offset is 0).", vmm_result);
		panic(fb);
		return;
	} else if (vmm_result == XDDOS_VMM_OUT_OF_MEMORY) {
		LOG_ERROR("KERNEL", "Failed to initialize Virtual Memory Manager! Error code 0x%x (Out of Memory).", vmm_result);
		panic(fb);
		return;
	} else if (vmm_result > 0) {
		LOG_ERROR("KERNEL", "Failed to initialize Virtual Memory Manager! Error code 0x%x (Unknown).", vmm_result);
		panic(fb);
		return;
	}

	// VMA init
	LOG_DEBUG("KERNEL", "Virtual Memory Allocator initializing...");
	xddos_vma_init(0xFFFFFFFF90000000ULL);

	// Allocate initial heap
	LOG_DEBUG("KERNEL", "Initial heap allocating...");
	size_t initial_heap_bytes = 128 * 1024;
	size_t initial_pages = initial_heap_bytes / PAGE_SIZE;

	void *initial_heap_block = xddos_vma_alloc_pages(initial_pages);
	if (!initial_heap_block) {
		LOG_ERROR("KERNEL", "Failed to allocate initial heap pages!");
		panic(fb);
		return;
	}

	xddos_memalloc_init(xddos_vma_alloc_pages, initial_heap_block, initial_heap_bytes);

	// Init fallback font
	LOG_INFO("KERNEL", "Initializing fallback font...");
	fallback_font = xddos_psf_init();

	xddos_graphics_clear(fb, 0);
	const char *msg = "xD-DOS (Extended Drive - Disk Operating System)\r\nMaintained by Jason Christian\r\n\r\n";
	xddos_graphics_clear(fb, 0x000000);
	xddos_graphics_psf_put_text(fb, fallback_font, msg, 4, 4, 0xFFFFFF, 0x000000);

	while (true) {
		xddos_pit_sleep_ms(1);
		xddos_graphics_psf_put_char(fb, fallback_font, inb(0x60), 4, 32, 0xFFFFFF, 0x000000);
	}

	// Halt
	LOG_INFO("KERNEL", "Nothing to do, halting...");
	hcf();
}