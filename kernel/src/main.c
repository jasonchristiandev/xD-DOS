#include "xddos/main.h"
#include "xddos/asm.h"
#include "xddos/graphics.h"
#include "xddos/kstdio.h"
#include "xddos/logging.h"
#include "xddos/pit.h"
#include "xddos/psf.h"
#include "xddos/syscallhandler.h"
#include "xddos/vma.h"
#include <stdint.h>
#include <stdlib.h>

extern uint64_t hhdm_offset;
extern xddos_psf_data_t *fallback_font;
extern xddos_framebuffer_t *fb;

void kernel_main() {
	LOG_DEBUG("VMM", "Done init.");

	// VMA init
	LOG_DEBUG("KERNEL", "Virtual Memory Allocator initializing...");
	xddos_vma_init(0xFFFFFFFFB0000000ULL);

	// Init syscall
	// LOG_DEBUG("KERNEL", "Initializing syscalls...");
	// xddos_syscall_init();

	// Demo
	xddos_graphics_clear(fb, 0);
	const char *msg = "xD-DOS (Extended Drive - Disk Operating System)\r\n> https://github.com/jasonchristiandev/xD-DOS\r\n> Maintained by Jason Christian.";
	xddos_graphics_clear(fb, 0x000000);
	xddos_graphics_psf_put_text(fb, fallback_font, msg, 4, 4, 0xFFFFFF, 0x000000);

	while (true) {
		xddos_pit_sleep_ms(1);
		uint8_t sc = inb(0x60);
		if (sc == 1) {
			volatile int x = 1;
			volatile int y = 0;
			x /= y;
		}
		char *str = malloc(7);
		xddos_kstdio_snprintf(str, 7, "%d   ", sc);
		xddos_graphics_psf_put_text(fb, fallback_font, str, 4, 52, 0xFFFFFF, 0x000000);
	}
}