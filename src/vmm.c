#include "xD-DOS/vmm.h"
#include "xD-DOS/memory.h" // IWYU pragma: keep
#include "xD-DOS/pmm.h"
#include <stdint.h>

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
		uint64_t new_phys = (uint64_t)pmm_alloc_page();
		page_table_t *new_virt = (page_table_t *) phys_to_virt(new_phys);
		memset(new_virt, 0, 4096);

		current_table->entries[pdpt_i] = new_phys | PTE_PRESENT | PTE_WRITABLE | PTE_USER;
	}

	current_table = (page_table_t *) phys_to_virt(current_table->entries[pdpt_i] & PTE_FRAME);

	// PD to PT
	if (!(current_table->entries[pd_i] & PTE_PRESENT)) {
		uint64_t new_phys = (uint64_t)pmm_alloc_page();
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
