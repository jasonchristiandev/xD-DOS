#include "xddos/vmm.h"
#include "xddos/asm.h"
#include "xddos/logging.h"
#include "xddos/main.h"
#include "xddos/pmm.h"
#include "xddos/requests.h"
#include <stddef.h>
#include <string.h>

typedef struct {
	uint64_t entries[512];
} __attribute__((packed)) xddos_vmm_page_table_t;

extern uint64_t hhdm_offset;
xddos_vmm_page_table_t *xddos_pml4;

static void alloc_entry(xddos_vmm_page_table_t *table, uint16_t idx) {
	uint64_t phys = (uint64_t) xddos_pmm_alloc_page();
	xddos_vmm_page_table_t *virt = (xddos_vmm_page_table_t *) (phys + hhdm_offset);
	memset(virt, 0, PAGE_SIZE);
	xddos_pmm_lock_region(phys, PAGE_SIZE);

	table->entries[idx] = phys | XDDOS_PTE_PRESENT | XDDOS_PTE_READWRITE;
}

xddos_vmm_init_result_t xddos_vmm_init() {
	// reuqests
	xddos_executable_address_t *exeaddr = xddos_request_executable_address();
	if (exeaddr == NULL) {
		LOG_ERROR("PMM", "Executable address request responded with NULL!");
		return XDDOS_VMM_INIT_NULL_RESPONSE;
	}
	xddos_executable_file_t *exefile = xddos_request_executable_file();
	if (exefile == NULL) {
		LOG_ERROR("PMM", "Executable file request responded with NULL!");
		return XDDOS_VMM_INIT_NULL_RESPONSE;
	}

	// allocate page table
	LOG_DEBUG("VMM", "Allocating PML4...");
	void *page = xddos_pmm_alloc_page();
	if (page == NULL) {
		return XDDOS_VMM_INIT_OUT_OF_MEMORY;
	}
	xddos_pml4 = (xddos_vmm_page_table_t *) (page + hhdm_offset);

	memset(xddos_pml4, 0, PAGE_SIZE);

	LOG_DEBUG("VMM", "Mapping reserved regions...");
	for (uint64_t i = 0; i < 0x100000000ULL; i += 0x200000) {
		xddos_vmm_map_table_huge(i + hhdm_offset, i, XDDOS_PTE_READWRITE);
	}
	for (uint64_t j = 0; j < exefile->size; j += PAGE_SIZE) {
		uint64_t addr = (uint64_t) exeaddr->phys + j;
		xddos_vmm_map_table(addr, addr, XDDOS_PTE_READWRITE);
	}
	for (uint64_t j = 0; j < exefile->size; j += PAGE_SIZE) {
		uint64_t phys = (uint64_t) exeaddr->phys + j;
		uint64_t virt = (uint64_t) exeaddr->virt + j;
		xddos_vmm_map_table(virt, phys, XDDOS_PTE_READWRITE);
	}
	xddos_vmm_map_table((uint64_t) xddos_pml4 - hhdm_offset, (uint64_t) xddos_pml4 - hhdm_offset, XDDOS_PTE_PRESENT | XDDOS_PTE_READWRITE);

	// create stack
	LOG_DEBUG("VMM", "Creating stack...");
	for (uint8_t i = 0; i < 8; i++) {
		xddos_vmm_map_table(STACK_BASE + i * PAGE_SIZE, (uint64_t) xddos_pmm_alloc_page(), XDDOS_PTE_PRESENT | XDDOS_PTE_READWRITE);
	}
	xddos_vmm_map_table(STACK_BASE + 8 * PAGE_SIZE, (uint64_t) xddos_pmm_alloc_page(), XDDOS_PTE_PRESENT | XDDOS_PTE_READWRITE); // to prevent silent error

	LOG_DEBUG("VMM", "Switching context...");
	__asm__ volatile("mov %0, %%cr3" ::"r"((uint64_t) xddos_pml4 - hhdm_offset) : "memory");

	LOG_DEBUG("VMM", "Switching stack...");
	uint64_t rsp = STACK_BASE + (8 * PAGE_SIZE);
	extern void xddos_vmm_switch_stack(uint64_t rsp, void (*entry_point)());
	xddos_vmm_switch_stack(rsp, kernel_main);

	return XDDOS_VMM_INIT_OK;
}

void xddos_vmm_map_table(uint64_t virt, uint64_t phys, uint64_t flags) {
	if ((virt & 0xFFF) != 0) return; // check alignment

	uint16_t pml4e_i = (virt >> 39) & 0b111111111;
	uint16_t pdpte_i = (virt >> 30) & 0b111111111;
	uint16_t pde_i = (virt >> 21) & 0b111111111;
	uint16_t pte_i = (virt >> 12) & 0b111111111;

	xddos_vmm_page_table_t *table = xddos_pml4;
	uint64_t entry;

	// pml4 to pdpt
	if (!(table->entries[pml4e_i] & XDDOS_PTE_PRESENT)) {
		alloc_entry(table, pml4e_i);
	}
	entry = table->entries[pml4e_i];
	table = (xddos_vmm_page_table_t *) (((uint64_t) entry & 0x000FFFFFFFFFF000) + hhdm_offset);

	// pdpt to pd
	if (!(table->entries[pdpte_i] & XDDOS_PTE_PRESENT)) {
		alloc_entry(table, pdpte_i);
	}
	entry = table->entries[pdpte_i];
	table = (xddos_vmm_page_table_t *) (((uint64_t) entry & 0x000FFFFFFFFFF000) + hhdm_offset);

	// pd to pt
	if (!(table->entries[pde_i] & XDDOS_PTE_PRESENT)) {
		alloc_entry(table, pde_i);
	}
	entry = table->entries[pde_i];
	table = (xddos_vmm_page_table_t *) (((uint64_t) entry & 0x000FFFFFFFFFF000) + hhdm_offset);

	// pt entry
	table->entries[pte_i] = phys | flags | XDDOS_PTE_PRESENT;

	invlpg((uint64_t) virt);
}

void xddos_vmm_map_table_huge(uint64_t virt, uint64_t phys, uint64_t flags) {
	if ((virt & 0xFFF) != 0) return; // check alignment

	uint16_t pml4e_i = (virt >> 39) & 0b111111111;
	uint16_t pdpte_i = (virt >> 30) & 0b111111111;
	uint16_t pde_i = (virt >> 21) & 0b111111111;

	xddos_vmm_page_table_t *table = xddos_pml4;
	uint64_t entry;

	// pml4 to pdpt
	if (!(table->entries[pml4e_i] & XDDOS_PTE_PRESENT)) {
		alloc_entry(table, pml4e_i);
	}
	entry = table->entries[pml4e_i];
	table = (xddos_vmm_page_table_t *) (((uint64_t) entry & 0x000FFFFFFFFFF000) + hhdm_offset);

	// pdpt to pd
	if (!(table->entries[pdpte_i] & XDDOS_PTE_PRESENT)) {
		alloc_entry(table, pdpte_i);
	}
	entry = table->entries[pdpte_i];
	table = (xddos_vmm_page_table_t *) (((uint64_t) entry & 0x000FFFFFFFFFF000) + hhdm_offset);

	// pd entry
	table->entries[pde_i] = (phys & 0x000FFFFFFFFFF000) | flags | XDDOS_PTE_PRESENT | XDDOS_PTE_PAGESIZE;

	invlpg((uint64_t) virt);
}
