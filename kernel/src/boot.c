#include "xddos/interrupts.h"
#include "xddos/logging.h"
#include "xddos/pmm.h"
#include "xddos/psf.h"
#include "xddos/requests.h"
#include "xddos/serial.h"
#include "xddos/vmm.h"
#include <stddef.h>

uint64_t hhdm_offset;
xddos_framebuffer_t *fb;
xddos_psf_data_t *fallback_font;

void boot_main() {
	if (xddos_request_base_revision_supported() == 0) {
		__asm__ volatile("hlt");
	}

	xddos_framebuffers_t *fbs = xddos_request_framebuffers();
	if (fbs == NULL || fbs->count < 1) __asm__ volatile("hlt");

	fb = fbs->framebuffers[0];

	xddos_serial_init();

	LOG_INFO("KERNEL", "Extended Drive - Disk Operating System (xD-DOS) Starting...");

	// HHDM
	xddos_hhdm_t *hhdm = xddos_request_hhdm();
	if (hhdm == NULL) {
		LOG_ERROR("PMM", "HHDM request responded with NULL!");
		xddos_panic(fb, "HHDM request responded with NULL!\r\nPlease refer to serial console for more information.");
		return;
	}
	hhdm_offset = hhdm->offset;

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
	if (vmm_result == XDDOS_VMM_INIT_NULL_RESPONSE) {
		LOG_ERROR("KERNEL", "Failed to initialize Virtual Memory Manager! Error code 0x%x (HHDM or Executable Address or Executable File Not Ready).", vmm_result);
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
}