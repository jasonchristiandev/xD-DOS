#include "xddos/main.h"
#include "xddos/graphics.h"
#include "xddos/logging.h"
#include "xddos/vma.h"

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

	for (;;) {}
}