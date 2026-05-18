//#include "xD-DOS/font.h"
#include "xD-DOS/memory.h"
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

static void hcf(void) {
	for (;;) {
		__asm__("hlt");
	}
}

void kernel_main(void) {
	if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
		hcf();
	}

	if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) hcf();

	struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
	volatile uint32_t *fb_ptr = fb->address;

	// uint32_t offset = 0;
	// for (;;) {
	//	for (size_t y = 0; y < fb->height; y++) {
	//		for (size_t x = 0; x < fb->width; x++) {
	//			uint32_t nX = ((x + offset) % fb->width) * 255 / fb->width;
	//			uint32_t nY = (fb->height - y) * 255 / fb->height;
	//			fb_ptr[y * (fb->pitch / 4) + x] = (nY << 8) | (nX << 16);
	//		}
	//	}
	//	offset++;
	//	offset %= fb->width;
	// }

	serial_init();
	serial_write_text("Hello world from serial COM1!");

	hcf();
}