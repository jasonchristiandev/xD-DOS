#include "xddos/gdt.h"
#include "xddos/logging.h"
#include <stdint.h>

__attribute__((aligned(8))) gdt_entry_t gdt[6];
gdt_pointer_t gdt_ptr;

extern void gdt_flush(uint64_t gdt_ptr);

void gdt_set_gate(int num, uint8_t access, uint8_t granularity) {
	gdt[num].base_low = 0;
	gdt[num].base_middle = 0;
	gdt[num].base_high = 0;
	gdt[num].limit_low = 0;

	gdt[num].access = access;
	gdt[num].granularity = granularity;
}

void gdt_init(void) {
	__asm__ __volatile__("cli");

	gdt_ptr.limit = sizeof(gdt_entry_t) * 6 - 1;
	gdt_ptr.base = (uint64_t) &gdt;

	// null descriptor
	gdt_set_gate(0, 0b00000000, 0b00000000);

	// code segment descriptor
	gdt_set_gate(1, 0b10011010, 0b00100000);

	// data segment descriptor
	gdt_set_gate(2, 0b10010010, 0b00000000);

	LOG_DEBUG("GDT", "Loading and flushing GDT...");
	gdt_flush((uint64_t) (&gdt_ptr));

	LOG_DEBUG("GDT", "Done init.");
}