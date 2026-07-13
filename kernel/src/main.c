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
extern psf_data_t *fallback_font;
extern requests_framebuffer_t *fb;

void kernel_main() {
	LOG_DEBUG("VMM", "Done init.");

	// init vma
	LOG_DEBUG("KERNEL", "Virtual Memory Allocator initializing...");
	vma_init();

	// init syscall
	// LOG_DEBUG("KERNEL", "Initializing syscalls...");
	// syscall_init();

	// simple keyboard and interrupt demo
	graphics_clear(fb, 0);
	const char *msg = "xD-DOS (Extended Drive - Disk Operating System)\r\n> https://github.com/jasonchristiandev/xD-DOS\r\n> Maintained by Jason Christian.";
	graphics_clear(fb, 0x000000);
	graphics_psf_put_text(fb, fallback_font, msg, 4, 4, 0xFFFFFF, 0x000000);

	char str[7];
	while (true) {
		pit_sleep_ms(1);
		uint8_t sc = inb(0x60);
		if (sc == 1) {
			volatile int x = 1;
			volatile int y = 0;
			x /= y;
		}
		memset(str, 0, 7);
		kstdio_snprintf(str, 7, "%d   ", sc);
		graphics_psf_put_text(fb, fallback_font, str, 4, 52, 0xFFFFFF, 0x000000);
	}
}