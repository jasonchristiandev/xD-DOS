// #include "xD-DOS/font.h"
#include "xD-DOS/logging.h"
#include "xD-DOS/pmm.h"
#include "xD-DOS/serial.h"
#include <limine.h>
#include <stddef.h>
#include <stdint.h>

extern volatile uint64_t limine_base_revision[];
extern volatile struct limine_framebuffer_request framebuffer_request;

static void hcf() {
	for (;;) {
		__asm__("hlt");
	}
}

void kernel_main() {
	if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == 0) {
		hcf();
	}

	if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) hcf();

	struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
	volatile uint32_t *fb_ptr = fb->address;

	serial_init();

	// PMM init
	LOG_INFO("PMM", "Physical Memory Manager initializing...");
	uint8_t pmm_result = pmm_init();
	if (pmm_result == 1) {
		LOG_ERROR("PMM", "Failed to initialize Physical Memory Manager! Error code 0x%x (Memory Map or HHDM Not Ready).", pmm_result);
		hcf();
	} else if (pmm_result == 2) {
		LOG_ERROR("PMM", "Failed to initialize Physical Memory Manager! Error code 0x%x (No Memory Available for Bitmap).", pmm_result);
		hcf();
	} else if (pmm_result > 0) {
		LOG_ERROR("PMM", "Failed to initialize Physical Memory Manager! Error code 0x%x (Unknown).", pmm_result);
		hcf();
	}
	
	DELETE_PREV_LINE();
	LOG_INFO("PMM", "Physical Memory Manager initialized.");

	// Halt
	LOG_INFO("KERNEL", "Nothing to do, halting...");
	hcf();
}