#include "xD-DOS/vmm.h"
#include "xD-DOS/logging.h"
#include "xD-DOS/memory.h" // IWYU pragma: keep
#include "xD-DOS/pmm.h"
#include <limine.h>
#include <stdint.h>

extern volatile struct limine_hhdm_request hhdm_request;
extern volatile struct limine_executable_address_request executable_address_request;

uint64_t hhdm_offset;
page_table_t *kernel_pml4 = NULL;

// Initializes VMM.
// Returns 1 if HHDM or executable address is unavailable.
// Returns 2 if HHDM offset is 0.
// Returns 3 if out of memory.
// Returns 0 otherwise.
uint8_t vmm_init(void) {
	struct limine_hhdm_response *hhdm = hhdm_request.response;
	struct limine_executable_address_response *exeaddr = executable_address_request.response;

	if (!hhdm || !exeaddr) return 1;

	hhdm_offset = hhdm->offset;
	if (hhdm_offset == 0) return 2;

	// Allocate physical page frame
	DEBUG_INFO("VMM", "Allocating physical page frame...");
	uint64_t pml4_phys = (uint64_t) pmm_alloc_page();
	if (pml4_phys == 0) return 3;
	kernel_pml4 = (page_table_t *) phys_to_virt(pml4_phys);
	DEBUG_INFO("VMM", "hhdm_offset: %llx", (unsigned long long) hhdm_offset);
	DEBUG_INFO("VMM", "pml4_phys:   %llx", (unsigned long long) pml4_phys);
	DEBUG_INFO("VMM", "kernel_pml4: %llx", (unsigned long long) kernel_pml4);

	DEBUG_INFO("VMM", "Zeroing out PML4 manually...");
	uint64_t *pml4_raw = (uint64_t *) kernel_pml4;
	for (uint16_t i = 0; i < 512; i++) {
		pml4_raw[i] = 0;
	}
	memset(kernel_pml4, 0, 4096);

	for (uint64_t i = 0; i < 0x100000000ULL; i += 4096) {
		vmm_map_table(kernel_pml4, (uint64_t) phys_to_virt(i), i, PTE_WRITABLE);
	}
	// for (uint64_t i = 0; i < 0x2000000ULL; i += 4096) {
	//	vmm_map_table(kernel_pml4, (uint64_t) phys_to_virt(i), i, PTE_WRITABLE);
	// }

	// Map the kernel code/data space
	DEBUG_INFO("VMM", "Mapping kernel code/data space...");
	uint64_t kernel_phys = exeaddr->physical_base;
	uint64_t kernel_virt = exeaddr->virtual_base;

	uint64_t kernel_size = 0x1000000;

	for (uint64_t offset = 0; offset < kernel_size; offset += 4096) {
		vmm_map_table(kernel_pml4, kernel_virt + offset, kernel_phys + offset, PTE_WRITABLE);
	}

	// Load new page tables directly into the CPU control register
	DEBUG_INFO("VMM", "Loading new page tables...");
	__asm__ volatile("mov %0, %%cr3" ::"r"(pml4_phys) : "memory");

	return 0;
}

uint8_t vmm_map_table(page_table_t *pml4, uint64_t virt, uint64_t phys, uint64_t flags) {
	uint16_t pml4_i = (virt >> 39) & 0x1FF;
	uint16_t pdpt_i = (virt >> 30) & 0x1FF;
	uint16_t pd_i = (virt >> 21) & 0x1FF;
	uint16_t pt_i = (virt >> 12) & 0x1FF;

	page_table_t *current_table = pml4;

	if (!(current_table->entries[pml4_i] & PTE_PRESENT)) {
		uint64_t new_phys = (uint64_t) pmm_alloc_page();
		page_table_t *new_virt = (page_table_t *) phys_to_virt(new_phys);
		memset(new_virt, 0, 4096);

		current_table->entries[pml4_i] = new_phys | PTE_PRESENT | PTE_WRITABLE | PTE_USER;
	}

	// Move pointer to the next level down
	current_table = (page_table_t *) phys_to_virt(current_table->entries[pml4_i] & PTE_FRAME);

	// PDPT to PD
	if (!(current_table->entries[pdpt_i] & PTE_PRESENT)) {
		uint64_t new_phys = (uint64_t) pmm_alloc_page();
		page_table_t *new_virt = (page_table_t *) phys_to_virt(new_phys);
		memset(new_virt, 0, 4096);

		current_table->entries[pdpt_i] = new_phys | PTE_PRESENT | PTE_WRITABLE | PTE_USER;
	}

	current_table = (page_table_t *) phys_to_virt(current_table->entries[pdpt_i] & PTE_FRAME);

	// PD to PT
	if (!(current_table->entries[pd_i] & PTE_PRESENT)) {
		uint64_t new_phys = (uint64_t) pmm_alloc_page();
		page_table_t *new_virt = (page_table_t *) phys_to_virt(new_phys);
		memset(new_virt, 0, 4096);

		current_table->entries[pd_i] = new_phys | PTE_PRESENT | PTE_WRITABLE | PTE_USER;
	}
	current_table = (page_table_t *) phys_to_virt(current_table->entries[pd_i] & PTE_FRAME);

	// Map the actual target address with the requested flags
	current_table->entries[pt_i] = (phys & PTE_FRAME) | flags | PTE_PRESENT;

	__asm__ volatile("invlpg (%0)" ::"r"(virt) : "memory");

	return 1;
}
