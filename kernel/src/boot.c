#include "xddos/gdt.h"
#include "xddos/interrupts.h"
#include "xddos/logging.h"
#include "xddos/pmm.h"
#include "xddos/psf.h"
#include "xddos/requests.h"
#include "xddos/serial.h"
#include "xddos/vmm.h"
#include "xddos/main.h"
#include <stddef.h>

uint64_t hhdm_offset;
requests_framebuffer_t *fb;
psf_data_t *fallback_font;

void boot_main() {
	__asm__ volatile("cli");
	if (request_base_revision_supported() == 0) {
		__asm__ volatile("hlt");
	}

	requests_framebuffers_t *fbs = request_framebuffers();
	if (fbs == NULL || fbs->count < 1) __asm__ volatile("hlt");

	fb = fbs->framebuffers[0];

	// init serial
	serial_init();

	LOG_INFO("KERNEL", "Extended Drive - Disk Operating System (xD-DOS) Starting...");

	// init hhdm
	requests_hhdm_t *hhdm = request_hhdm();
	if (hhdm == NULL) {
		LOG_ERROR("PMM", "HHDM request responded with NULL!");
		interrupts_panic(fb, "HHDM request responded with NULL!\r\nPlease refer to serial console for more information.");
		return;
	}
	hhdm_offset = hhdm->offset;

	// init fallback font
	LOG_DEBUG("KERNEL", "Initializing fallback font...");
	fallback_font = psf_init();

	// init gdt
	LOG_DEBUG("KERNEL", "Initializing GDT (Global Descriptor Table)...");
	gdt_init();

	// init idt
	LOG_DEBUG("KERNEL", "Initializing IDT (Interrupt Descriptor Table)...");
	interrupts_init();

	// init pmm
	LOG_DEBUG("KERNEL", "Physical Memory Manager initializing...");
	pmm_init_result_t pmm_result = pmm_init();
	if (pmm_result == PMM_INIT_NULL_RESPONSE) {
		LOG_ERROR("KERNEL", "Failed to initialize Physical Memory Manager! Error code 0x%x (Memory Map or HHDM Not Ready).", pmm_result);
		interrupts_panic(fb, "Failed to initialize Physical Memory Manager!\r\nPlease refer to serial console for more information.");
		return;
	} else if (pmm_result == PMM_INIT_OUT_OF_SPACE) {
		LOG_ERROR("KERNEL", "Failed to initialize Physical Memory Manager! Error code 0x%x (No Memory Available for Bitmap).", pmm_result);
		interrupts_panic(fb, "Failed to initialize Physical Memory Manager!\r\nPlease refer to serial console for more information.");
		return;
	} else if (pmm_result > 0) {
		LOG_ERROR("KERNEL", "Failed to initialize Physical Memory Manager! Error code 0x%x (Unknown).", pmm_result);
		interrupts_panic(fb, "Failed to initialize Physical Memory Manager!\r\nPlease refer to serial console for more information.");
		return;
	}

	// init pmm
	LOG_DEBUG("KERNEL", "Virtual Memory Manager initializing...");
	vmm_init_result_t vmm_result = vmm_init();
	if (vmm_result == VMM_INIT_NULL_RESPONSE) {
		LOG_ERROR("KERNEL", "Failed to initialize Virtual Memory Manager! Error code 0x%x (HHDM or Executable Address or Executable File Not Ready).", vmm_result);
		interrupts_panic(fb, "Failed to initialize Virtual Memory Manager!\r\nPlease refer to serial console for more information.");
		return;
	} else if (vmm_result == VMM_INIT_OUT_OF_MEMORY) {
		LOG_ERROR("KERNEL", "Failed to initialize Virtual Memory Manager! Error code 0x%x (Out of Memory).", vmm_result);
		interrupts_panic(fb, "Failed to initialize Virtual Memory Manager!\r\nPlease refer to serial console for more information.");
		return;
	} else if (vmm_result > 0) {
		LOG_ERROR("KERNEL", "Failed to initialize Virtual Memory Manager! Error code 0x%x (Unknown).", vmm_result);
		interrupts_panic(fb, "Failed to initialize Virtual Memory Manager!\r\nPlease refer to serial console for more information.");
		return;
	}

	// jump to kernel_main (hopefully)
}