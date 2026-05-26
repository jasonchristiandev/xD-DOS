#include "xddos/vmm.h"
#include "xddos/logging.h"
#include "xddos/memory.h" // IWYU pragma: keep
#include "xddos/pmm.h"
#include "xddos/requests.h"
#include <stdint.h>

uint64_t hhdm_offset;
vmm_page_table_t *kernel_pml4 = NULL;

// Initializes VMM.
// Returns 1 if HHDM or executable address or executable file is unavailable.
// Returns 2 if HHDM offset is 0.
// Returns 3 if out of memory.
// Returns 0 otherwise.
uint8_t vmm_init(void) {
	LOG_DEBUG("VMM", "Checking responses...");
	xddos_hhdm_t *hhdm = request_hhdm();
	xddos_executable_address_t *exeaddr = request_executable_address();
	xddos_executable_file_t *exefile = request_executable_file();

	if (!hhdm || !exeaddr || !exefile) {
		LOG_ERROR("VMM", "HHDM or executable address or executable file is NULL!");
		return 1;
	}

	hhdm_offset = hhdm->offset;
	if (hhdm_offset == 0) {
		LOG_ERROR("VMM", "HHDM offset is 0!");
		return 2;
	}

	// Allocate physical page frame
	LOG_DEBUG("VMM", "Allocating physical page frame...");
	uint64_t pml4_phys = (uint64_t) pmm_alloc_page();
	if (pml4_phys == 0) return 3;
	kernel_pml4 = (vmm_page_table_t *) MEMORY_PHYS_TO_VIRT(pml4_phys);
	LOG_DEBUG("VMM", "  hhdm_offset: 0x%llx", (unsigned long long) hhdm_offset);
	LOG_DEBUG("VMM", "  pml4_phys:   0x%llx", (unsigned long long) pml4_phys);
	LOG_DEBUG("VMM", "  kernel_pml4: 0x%llx", (unsigned long long) kernel_pml4);
	LOG_DEBUG("VMM", "Allocated physical page frame.");

	LOG_DEBUG("VMM", "Zeroing out PML4...");
	uint64_t *pml4_raw = (uint64_t *) kernel_pml4;
	memset(kernel_pml4, 0, 4096);

	for (uint64_t i = 0; i < 0x100000000ULL; i += 4096) {
		vmm_map_table(kernel_pml4, (uint64_t) MEMORY_PHYS_TO_VIRT(i), i, MEMORY_PTE_WRITABLE);
	}
	// for (uint64_t i = 0; i < 0x2000000ULL; i += 4096) {
	//	vmm_map_table(kernel_pml4, (uint64_t) MEMORY_PHYS_TO_VIRT(i), i, MEMORY_PTE_WRITABLE);
	// }

	// Map the kernel code/data space
	LOG_DEBUG("VMM", "Mapping kernel code/data space...");
	uint64_t kernel_phys = exeaddr->phys;
	uint64_t kernel_virt = exeaddr->virt;

	// uint64_t kernel_size = 0x1000000;
	uint64_t kernel_size = exefile->size;

	LOG_DEBUG("VMM", "Kernel size: %lld bytes.", kernel_size);

	for (uint64_t offset = 0; offset < kernel_size; offset += 4096) {
		vmm_map_table(kernel_pml4, kernel_virt + offset, kernel_phys + offset, MEMORY_PTE_WRITABLE);
	}

	// Load new page tables directly into the CPU control register
	LOG_DEBUG("VMM", "Loading new page tables...");
	__asm__ volatile("mov %0, %%cr3" ::"r"(pml4_phys) : "memory");
	LOG_DEBUG("VMM", "Escaped bootloader memory.");

	return 0;
}

uint8_t vmm_map_table(vmm_page_table_t *pml4, uint64_t virt, uint64_t phys, uint64_t flags) {
	uint16_t pml4_i = (virt >> 39) & 0x1FF;
	uint16_t pdpt_i = (virt >> 30) & 0x1FF;
	uint16_t pd_i = (virt >> 21) & 0x1FF;
	uint16_t pt_i = (virt >> 12) & 0x1FF;

	vmm_page_table_t *current_table = pml4;

	if (!(current_table->entries[pml4_i] & MEMORY_PTE_PRESENT)) {
		uint64_t new_phys = (uint64_t) pmm_alloc_page();
		vmm_page_table_t *new_virt = (vmm_page_table_t *) MEMORY_PHYS_TO_VIRT(new_phys);
		memset(new_virt, 0, 4096);

		current_table->entries[pml4_i] = new_phys | MEMORY_PTE_PRESENT | MEMORY_PTE_WRITABLE;
	}

	// Move pointer to the next level down
	current_table = (vmm_page_table_t *) MEMORY_PHYS_TO_VIRT(current_table->entries[pml4_i] & MEMORY_PTE_FRAME);

	// PDPT to PD
	if (!(current_table->entries[pdpt_i] & MEMORY_PTE_PRESENT)) {
		uint64_t new_phys = (uint64_t) pmm_alloc_page();
		vmm_page_table_t *new_virt = (vmm_page_table_t *) MEMORY_PHYS_TO_VIRT(new_phys);
		memset(new_virt, 0, 4096);

		current_table->entries[pdpt_i] = new_phys | MEMORY_PTE_PRESENT | MEMORY_PTE_WRITABLE | MEMORY_PTE_USER;
	}

	current_table = (vmm_page_table_t *) MEMORY_PHYS_TO_VIRT(current_table->entries[pdpt_i] & MEMORY_PTE_FRAME);

	// PD to PT
	if (!(current_table->entries[pd_i] & MEMORY_PTE_PRESENT)) {
		uint64_t new_phys = (uint64_t) pmm_alloc_page();
		vmm_page_table_t *new_virt = (vmm_page_table_t *) MEMORY_PHYS_TO_VIRT(new_phys);
		memset(new_virt, 0, 4096);

		current_table->entries[pd_i] = new_phys | MEMORY_PTE_PRESENT | MEMORY_PTE_WRITABLE | MEMORY_PTE_USER;
	}
	current_table = (vmm_page_table_t *) MEMORY_PHYS_TO_VIRT(current_table->entries[pd_i] & MEMORY_PTE_FRAME);

	// Map the actual target address with the requested flags
	current_table->entries[pt_i] = (phys & MEMORY_PTE_FRAME) | flags | MEMORY_PTE_PRESENT;

	__asm__ volatile("invlpg (%0)" ::"r"(virt) : "memory");

	return 1;
}
