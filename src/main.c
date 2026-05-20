// #include "xD-DOS/font.h"
#include "xD-DOS/logging.h"
#include "xD-DOS/pit.h"
#include "xD-DOS/pmm.h"
#include "xD-DOS/requests.h"
#include "xD-DOS/serial.h"
#include "xD-DOS/vmm.h"
#include <stddef.h>
#include <stdint.h>

static void hcf() {
	for (;;) {
		__asm__("hlt");
	}
}

static void panic(xD_DOS_framebuffer *fb) {
	const uint8_t timeout = 5;
	LOG_ERROR("KERNEL", "Kernel panic! Exiting in %d...", timeout);
	sleep_s(timeout);
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
	LOG_INFO("PMM", "Physical Memory Manager initializing...");
	uint8_t pmm_result = pmm_init();
	if (pmm_result == 1) {
		LOG_ERROR("PMM", "Failed to initialize Physical Memory Manager! Error code 0x%x (Memory Map or HHDM Not Ready).", pmm_result);
		panic(fb);
	} else if (pmm_result == 2) {
		LOG_ERROR("PMM", "Failed to initialize Physical Memory Manager! Error code 0x%x (No Memory Available for Bitmap).", pmm_result);
		panic(fb);
	} else if (pmm_result > 0) {
		LOG_ERROR("PMM", "Failed to initialize Physical Memory Manager! Error code 0x%x (Unknown).", pmm_result);
		panic(fb);
	}
	DELETE_PREV_LINE();
	LOG_INFO("PMM", "Physical Memory Manager initialized.");

	// VMM init
	LOG_INFO("VMM", "Virtual Memory Manager initializing...");
	uint8_t vmm_result = vmm_init();
	DELETE_PREV_LINE();
	LOG_INFO("VMM", "Virtual Memory Manager initialized.");

	// Halt
	LOG_INFO("KERNEL", "Nothing to do, halting...");
	panic(fb); // panic testing
}