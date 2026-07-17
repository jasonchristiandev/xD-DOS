#include "xddos/asm.h"
#include "xddos/interrupts.h"
#include "xddos/logging.h"
#include "xddos/main.h"
#include "xddos/pmm.h"
#include "xddos/requests.h"
#include "xddos/serial.h"
#include "xddos/vmm.h"
#include <stddef.h>

uint64_t hhdm_offset;

void boot_main() {
	if (request_base_revision_supported() == 0) hlt();

	// init serial
	serial_init();

	LOG_INFO("KERNEL", "Extended Drive - Disk Operating System (xD-DOS) Starting...");

	// init hhdm
	requests_hhdm_t *hhdm = request_hhdm();
	if (hhdm == NULL) {
		interrupts_fail("HHDM request responded with NULL!", 1, "HHDM_NULL");
	}
	hhdm_offset = hhdm->offset;

	// init pmm
	LOG_DEBUG("KERNEL", "Physical Memory Manager initializing...");
	pmm_init_result_t pmm_result = pmm_init();
	char *pmm_result_name[3] = {
		[PMM_INIT_OK] = "OK",
		[PMM_INIT_NULL_RESPONSE] = "NULL_RESPONSE",
		[PMM_INIT_OUT_OF_SPACE] = "OUT_OF_SPACE"};
	if (pmm_result != 0) {
		char *name = "UNKNOWN";
		if (pmm_result < 3) name = pmm_result_name[pmm_result];
		interrupts_fail("PMM_INIT bad return!", pmm_result, name);
	}

	// init vmm
	LOG_DEBUG("KERNEL", "Virtual Memory Manager initializing...");
	vmm_init_result_t vmm_result = vmm_init();
	char *vmm_result_name[3] = {
		[VMM_INIT_OK] = "OK",
		[VMM_INIT_NULL_RESPONSE] = "NULL_RESPONSE",
		[VMM_INIT_OUT_OF_MEMORY] = "OUT_OF_SPACE"};
	if (vmm_result != 0) {
		char *name = "UNKNOWN";
		if (vmm_result < 3) name = vmm_result_name[vmm_result];
		interrupts_fail("VMM_INIT bad return!", vmm_result, name);
	}

	// jump to kernel_main (hopefully)
}