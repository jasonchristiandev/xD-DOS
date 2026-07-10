#include "xddos/main.h"
#include "xddos/asm.h"
#include "xddos/gdt.h"
#include "xddos/graphics.h"
#include "xddos/interrupts.h"
#include "xddos/kstdio.h"
#include "xddos/logging.h"
#include "xddos/memalloc.h"
#include "xddos/pit.h"
#include "xddos/pmm.h"
#include "xddos/psf.h"
#include "xddos/syscallhandler.h"
#include "xddos/vma.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

extern uint64_t hhdm_offset;
extern xddos_psf_data_t *fallback_font;
extern xddos_framebuffer_t *fb;

void kernel_main() {
	LOG_DEBUG("VMM", "Done init.");

	// VMA init
	LOG_DEBUG("KERNEL", "Virtual Memory Allocator initializing...");
	xddos_vma_init(0xFFFFFFFF90000000ULL);

	// Allocate initial heap
	LOG_DEBUG("KERNEL", "Initial heap allocating...");
	size_t initial_heap_bytes = 128 * 1024;
	size_t initial_pages = initial_heap_bytes / PAGE_SIZE;

	void *initial_heap_block = xddos_vma_alloc_pages(initial_pages);
	if (!initial_heap_block) {
		LOG_ERROR("KERNEL", "Failed to allocate initial heap pages!");
		xddos_panic(fb, "Failed to initialize Virtual Memory Manager!\r\nPlease refer to serial console for more information.");
		return;
	}

	xddos_memalloc_init(xddos_vma_alloc_pages, initial_heap_block, initial_heap_bytes);

	// Init GDT
	LOG_DEBUG("KERNEL", "Initializing GDT (Global Descriptor Table)...");
	xddos_gdt_init();

	// Init syscall
	LOG_DEBUG("KERNEL", "Initializing syscalls...");
	xddos_syscall_init();

	// Init fallback font
	LOG_DEBUG("KERNEL", "Initializing fallback font...");
	fallback_font = xddos_psf_init();

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

	// Halt
	LOG_INFO("KERNEL", "Nothing to do, halting...");
	__asm__ volatile("hlt");
}