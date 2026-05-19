// #include "xD-DOS/font.h"
#include "xD-DOS/logging.h"
#include "xD-DOS/pit.h"
#include "xD-DOS/pmm.h"
#include "xD-DOS/serial.h"
#include "xD-DOS/vmm.h"
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

static void panic(struct limine_framebuffer *fb) {
	const uint8_t timeout = 5;
	LOG_ERROR("KERNEL", "Kernel panic! Exiting in %d...", timeout);
	volatile uint32_t *fb_ptr = fb->address;
	uint8_t text[16][33] = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
							{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
							{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
							{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
							{0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0},
							{0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1},
							{0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1},
							{0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0},
							{0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0},
							{0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0},
							{0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0},
							{0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0},
							{0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0},
							{0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1},
							{0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1},
							{0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0}};
	for (int i = timeout * 4; i >= 1; i--) {
		for (size_t y = 0; y < fb->height; y++) {
			for (size_t x = 0; x < fb->width; x++) {
				fb_ptr[y * (fb->pitch / 4) + x] = 0xFF0000;
				size_t px = (x + y / 4) / 4 + 3;
				size_t py = y / 2 - 5;
				px %= 33;
				py %= 16;
				if (y - 10 <= i * 32 && text[py][px]) {
					fb_ptr[y * (fb->pitch / 4) + x] = 0xFFFFFF;
				}
			}
		}
		sleep_ms(250);
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