// Taken from wiki.osdev.org

#include <limine.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "xD-DOS/font.h"

__attribute__((used, section(".limine_requests"))) static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(6);
__attribute__((used, section(".limine_requests"))) static volatile struct limine_framebuffer_request framebuffer_request = {
	.id = LIMINE_FRAMEBUFFER_REQUEST_ID,
	.revision = 0};
__attribute__((used, section(".limine_requests_start"))) static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;
__attribute__((used, section(".limine_requests_end"))) static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

void *memcpy(void *restrict dest, const void *restrict src, size_t n) {
	uint8_t *restrict pdest = dest;
	const uint8_t *restrict psrc = src;

	for (size_t i = 0; i < n; i++) {
		pdest[i] = psrc[i];
	}

	return dest;
}

void *memset(void *s, int c, size_t n) {
	uint8_t *p = s;

	for (size_t i = 0; i < n; i++) {
		p[i] = (uint8_t) c;
	}

	return s;
}

void *memmove(void *dest, const void *src, size_t n) {
	uint8_t *pdest = dest;
	const uint8_t *psrc = src;

	if ((uintptr_t) src > (uintptr_t) dest) {
		for (size_t i = 0; i < n; i++) {
			pdest[i] = psrc[i];
		}
	} else if ((uintptr_t) src < (uintptr_t) dest) {
		for (size_t i = n; i > 0; i--) {
			pdest[i - 1] = psrc[i - 1];
		}
	}

	return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
	const uint8_t *p1 = s1;
	const uint8_t *p2 = s2;

	for (size_t i = 0; i < n; i++) {
		if (p1[i] != p2[i]) {
			return p1[i] < p2[i] ? -1 : 1;
		}
	}

	return 0;
}

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

	
}