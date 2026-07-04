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
#include "xddos/requests.h"
#include "xddos/serial.h"
#include "xddos/syscallhandler.h"
#include "xddos/vma.h"
#include "xddos/vmm.h"
#include <stddef.h>
#include <stdlib.h>

xddos_psf_data_t *fallback_font;

uint64_t syscall(uint64_t vector_id, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
	uint64_t ret;
	__asm__ __volatile__(
		"mov %1, %%rax\n\t"
		"mov %2, %%rdi\n\t"
		"mov %3, %%rsi\n\t"
		"mov %4, %%rdx\n\t"
		"syscall\n\t"
		"mov %%rax, %0"
		: "=r"(ret)
		: "r"(vector_id), "r"(arg1), "r"(arg2), "r"(arg3)
		: "rax", "rdi", "rsi", "rdx", "rcx", "r11", "memory");
	return ret;
}

void kernel_main() {
	if (xddos_request_base_revision_supported() == 0) {
		__asm__ volatile("hlt");
	}

	xddos_framebuffers_t *fbs = xddos_request_framebuffers();
	if (fbs == NULL || fbs->count < 1) __asm__ volatile("hlt");

	xddos_framebuffer_t *fb = fbs->framebuffers[0];

	xddos_serial_init();

	LOG_INFO("KERNEL", "Extended Drive - Disk Operating System (xD-DOS) Starting...");

	// Interrupts
	LOG_DEBUG("KERNEL", "Initializing IDT (Interrupt Descriptor Table)...");
	xddos_interrupts_init();

	// PMM init
	LOG_DEBUG("KERNEL", "Physical Memory Manager initializing...");
	xddos_pmm_init_result_t pmm_result = xddos_pmm_init();
	if (pmm_result == XDDOS_PMM_INIT_NULL_RESPONSE) {
		LOG_ERROR("KERNEL", "Failed to initialize Physical Memory Manager! Error code 0x%x (Memory Map or HHDM Not Ready).", pmm_result);
		xddos_panic(fb, "Failed to initialize Physical Memory Manager!\r\nPlease refer to serial console for more information.");
		return;
	} else if (pmm_result == XDDOS_PMM_INIT_OUT_OF_SPACE) {
		LOG_ERROR("KERNEL", "Failed to initialize Physical Memory Manager! Error code 0x%x (No Memory Available for Bitmap).", pmm_result);
		xddos_panic(fb, "Failed to initialize Physical Memory Manager!\r\nPlease refer to serial console for more information.");
		return;
	} else if (pmm_result > 0) {
		LOG_ERROR("KERNEL", "Failed to initialize Physical Memory Manager! Error code 0x%x (Unknown).", pmm_result);
		xddos_panic(fb, "Failed to initialize Physical Memory Manager!\r\nPlease refer to serial console for more information.");
		return;
	}

	// VMM init
	LOG_DEBUG("KERNEL", "Virtual Memory Manager initializing...");
	xddos_vmm_init_result_t vmm_result = xddos_vmm_init();
	if (vmm_result == XDDOS_VMM_INIT_NO_RESPONSES) {
		LOG_ERROR("KERNEL", "Failed to initialize Virtual Memory Manager! Error code 0x%x (HHDM or Executable Address or Executable File Not Ready).", vmm_result);
		xddos_panic(fb, "Failed to initialize Virtual Memory Manager!\r\nPlease refer to serial console for more information.");
		return;
	} else if (vmm_result == XDDOS_VMM_INIT_OFFSET_ZERO) {
		LOG_ERROR("KERNEL", "Failed to initialize Virtual Memory Manager! Error code 0x%x (HHDM Offset is 0).", vmm_result);
		xddos_panic(fb, "Failed to initialize Virtual Memory Manager!\r\nPlease refer to serial console for more information.");
		return;
	} else if (vmm_result == XDDOS_VMM_INIT_OUT_OF_MEMORY) {
		LOG_ERROR("KERNEL", "Failed to initialize Virtual Memory Manager! Error code 0x%x (Out of Memory).", vmm_result);
		xddos_panic(fb, "Failed to initialize Virtual Memory Manager!\r\nPlease refer to serial console for more information.");
		return;
	} else if (vmm_result > 0) {
		LOG_ERROR("KERNEL", "Failed to initialize Virtual Memory Manager! Error code 0x%x (Unknown).", vmm_result);
		xddos_panic(fb, "Failed to initialize Virtual Memory Manager!\r\nPlease refer to serial console for more information.");
		return;
	}

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