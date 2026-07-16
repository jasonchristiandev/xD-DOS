#include "xddos/vmm.h"
#include "xddos/asm.h"
#include "xddos/logging.h"
#include "xddos/main.h"
#include "xddos/pmm.h"
#include "xddos/requests.h"
#include <stddef.h>
#include <string.h>

extern void vmm_switch_stack(uint64_t rsp, void (*entry_point)());

typedef struct {
	uint64_t entries[512];
} __attribute__((packed)) vmm_page_table_t;

vmm_page_table_t *pml4;

static void alloc_entry(vmm_page_table_t *table, uint16_t idx) {
	uint64_t phys = (uint64_t) pmm_alloc_page();
	vmm_page_table_t *virt = (vmm_page_table_t *) (phys + hhdm_offset);
	memset(virt, 0, PAGE_SIZE);
	pmm_lock_region(phys, PAGE_SIZE);

	table->entries[idx] = phys | PTE_PRESENT | PTE_READWRITE;
}

vmm_init_result_t vmm_init() {
	// reuqests
	requests_executable_address_t *exeaddr = request_executable_address();
	if (exeaddr == NULL) {
		LOG_ERROR("PMM", "Executable address request responded with NULL!");
		return VMM_INIT_NULL_RESPONSE;
	}
	requests_executable_file_t *exefile = request_executable_file();
	if (exefile == NULL) {
		LOG_ERROR("PMM", "Executable file request responded with NULL!");
		return VMM_INIT_NULL_RESPONSE;
	}

	// allocate page table
	LOG_DEBUG("VMM", "Allocating PML4...");
	void *page = pmm_alloc_page();
	if (page == NULL) {
		return VMM_INIT_OUT_OF_MEMORY;
	}
	pml4 = (vmm_page_table_t *) (page + hhdm_offset);

	memset(pml4, 0, PAGE_SIZE);

	LOG_DEBUG("VMM", "Mapping reserved regions...");
	for (uint64_t i = 0; i < 0x100000000ULL; i += 0x200000) {
		vmm_map_table_huge(i + hhdm_offset, i, PTE_READWRITE);
	}
	for (uint64_t j = 0; j < exefile->size; j += PAGE_SIZE) {
		uint64_t addr = (uint64_t) exeaddr->phys + j;
		vmm_map_table(addr, addr, PTE_READWRITE);
	}
	for (uint64_t j = 0; j < exefile->size; j += PAGE_SIZE) {
		uint64_t phys = (uint64_t) exeaddr->phys + j;
		uint64_t virt = (uint64_t) exeaddr->virt + j;
		vmm_map_table(virt, phys, PTE_READWRITE);
	}
	vmm_map_table((uint64_t) pml4 - hhdm_offset, (uint64_t) pml4 - hhdm_offset, PTE_PRESENT | PTE_READWRITE);

	// create stack
	LOG_DEBUG("VMM", "Creating stack...");
	for (uint8_t i = 0; i < 8; i++) {
		vmm_map_table(STACK_BASE + i * PAGE_SIZE, (uint64_t) pmm_alloc_page(), PTE_PRESENT | PTE_READWRITE);
	}
	vmm_map_table(STACK_BASE + 8 * PAGE_SIZE, (uint64_t) pmm_alloc_page(), PTE_PRESENT | PTE_READWRITE); // to prevent silent error

	LOG_DEBUG("VMM", "Switching context...");
	__asm__ __volatile__("mov %0, %%cr3" ::"r"((uint64_t) pml4 - hhdm_offset) : "memory");

	LOG_DEBUG("VMM", "Switching stack...");
	uint64_t rsp = STACK_BASE + (8 * PAGE_SIZE);
	vmm_switch_stack(rsp, kernel_main);

	return VMM_INIT_OK;
}

void vmm_map_table(uint64_t virt, uint64_t phys, uint64_t flags) {
	if ((virt & 0xFFF) != 0) return; // check alignment

	uint16_t pml4e_i = (virt >> 39) & 0b111111111;
	uint16_t pdpte_i = (virt >> 30) & 0b111111111;
	uint16_t pde_i = (virt >> 21) & 0b111111111;
	uint16_t pte_i = (virt >> 12) & 0b111111111;

	vmm_page_table_t *table = pml4;
	uint64_t entry;

	// pml4 to pdpt
	if (!(table->entries[pml4e_i] & PTE_PRESENT)) {
		alloc_entry(table, pml4e_i);
	}
	entry = table->entries[pml4e_i];
	table = (vmm_page_table_t *) (((uint64_t) entry & 0x000FFFFFFFFFF000) + hhdm_offset);

	// pdpt to pd
	if (!(table->entries[pdpte_i] & PTE_PRESENT)) {
		alloc_entry(table, pdpte_i);
	}
	entry = table->entries[pdpte_i];
	table = (vmm_page_table_t *) (((uint64_t) entry & 0x000FFFFFFFFFF000) + hhdm_offset);

	// pd to pt
	if (!(table->entries[pde_i] & PTE_PRESENT)) {
		alloc_entry(table, pde_i);
	}
	entry = table->entries[pde_i];
	table = (vmm_page_table_t *) (((uint64_t) entry & 0x000FFFFFFFFFF000) + hhdm_offset);

	// pt entry
	table->entries[pte_i] = phys | flags | PTE_PRESENT;

	invlpg((uint64_t) virt);
}

void vmm_map_table_huge(uint64_t virt, uint64_t phys, uint64_t flags) {
	if ((virt & 0xFFF) != 0) return; // check alignment

	uint16_t pml4e_i = (virt >> 39) & 0b111111111;
	uint16_t pdpte_i = (virt >> 30) & 0b111111111;
	uint16_t pde_i = (virt >> 21) & 0b111111111;

	vmm_page_table_t *table = pml4;
	uint64_t entry;

	// pml4 to pdpt
	if (!(table->entries[pml4e_i] & PTE_PRESENT)) {
		alloc_entry(table, pml4e_i);
	}
	entry = table->entries[pml4e_i];
	table = (vmm_page_table_t *) (((uint64_t) entry & 0x000FFFFFFFFFF000) + hhdm_offset);

	// pdpt to pd
	if (!(table->entries[pdpte_i] & PTE_PRESENT)) {
		alloc_entry(table, pdpte_i);
	}
	entry = table->entries[pdpte_i];
	table = (vmm_page_table_t *) (((uint64_t) entry & 0x000FFFFFFFFFF000) + hhdm_offset);

	// pd entry
	table->entries[pde_i] = (phys & 0x000FFFFFFFFFF000) | flags | PTE_PRESENT | PTE_PAGESIZE;

	invlpg((uint64_t) virt);
}
