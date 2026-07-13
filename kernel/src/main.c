#include "xddos/main.h"
#include "xddos/asm.h"
#include "xddos/graphics.h"
#include "xddos/kstdio.h"
#include "xddos/logging.h"
#include "xddos/pit.h"
#include "xddos/psf.h"
#include "xddos/vma.h"
#include <stdint.h>
#include <string.h>

extern uint64_t hhdm_offset;
extern xddos_psf_data_t *fallback_font;
extern xddos_framebuffer_t *fb;

void kernel_main() {
	LOG_DEBUG("VMM", "Done init.");

	// VMA init
	LOG_DEBUG("KERNEL", "Virtual Memory Allocator initializing...");
	xddos_vma_init();

	// VMA test
	void *ptr1 = xddos_vma_malloc(1024);
	LOG_DEBUG("KERNEL", "1kb test: 0x%llx", ptr1);

	uint8_t *ptr2 = xddos_vma_malloc(2 * 1024 * 1024);
	LOG_DEBUG("KERNEL", "overflow test (2mb): 0x%llx", ptr2);

	if (ptr2) {
		ptr2[0] = 0xAA;
		ptr2[2 * 1024 * 1024 - 1] = 0xBB;

		LOG_DEBUG("KERNEL", "ptr2[0]: 0x%llx, ptr2[2 * 1024 * 1024 - 1]: 0x%llx",
				  ptr2[0], ptr2[2 * 1024 * 1024 - 1]);
	}

	xddos_vma_free(ptr1);
	xddos_vma_free(ptr2);
	LOG_DEBUG("KERNEL", "free ptr1 ptr2");

	void *ptr3 = xddos_vma_malloc(2048);
	LOG_DEBUG("KERNEL", "free merge test (should be same as ptr1): 0x%llx", (uint64_t) ptr3);

	// Init syscall
	// LOG_DEBUG("KERNEL", "Initializing syscalls...");
	// xddos_syscall_init();

	// Demo
	xddos_graphics_clear(fb, 0);
	const char *msg = "xD-DOS (Extended Drive - Disk Operating System)\r\n> https://github.com/jasonchristiandev/xD-DOS\r\n> Maintained by Jason Christian.";
	xddos_graphics_clear(fb, 0x000000);
	xddos_graphics_psf_put_text(fb, fallback_font, msg, 4, 4, 0xFFFFFF, 0x000000);

	char str[7];
	while (true) {
		xddos_pit_sleep_ms(1);
		uint8_t sc = inb(0x60);
		if (sc == 1) {
			volatile int x = 1;
			volatile int y = 0;
			x /= y;
		}
		memset(str, 0, 7);
		xddos_kstdio_snprintf(str, 7, "%d   ", sc);
		xddos_graphics_psf_put_text(fb, fallback_font, str, 4, 52, 0xFFFFFF, 0x000000);
	}
}