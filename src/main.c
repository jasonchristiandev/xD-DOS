// #include "xD-DOS/font.h"
#include "xD-DOS/logging.h"
#include "xD-DOS/pmm.h"
#include "xD-DOS/serial.h"
#include <limine.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

__attribute__((used, section(".limine_requests"))) static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(6);
__attribute__((used, section(".limine_requests"))) static volatile struct limine_framebuffer_request framebuffer_request = {
	.id = LIMINE_FRAMEBUFFER_REQUEST_ID,
	.revision = 0};
__attribute__((used, section(".limine_requests"))) static volatile struct limine_memmap_request memmap_request = {
	.id = LIMINE_MEMMAP_REQUEST_ID,
	.revision = 0};
__attribute__((used, section(".limine_requests_start"))) static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;
__attribute__((used, section(".limine_requests_end"))) static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

static void hcf() {
	for (;;) {
		__asm__("hlt");
	}
}

void kernel_main() {
	if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
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