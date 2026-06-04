#include "xddos/gdt.h"
#include <stdint.h>

__attribute__((aligned(8))) xddos_gdt_entry_t gdt[6];
__attribute__((aligned(8))) xddos_gdt_ptr_t gp;

extern void xddos_gdt_flush();

void xddos_gdt_set_gate(int num, uint8_t access, uint8_t gran) {
	gdt[num].base_low = 0;
	gdt[num].base_middle = 0;
	gdt[num].base_high = 0;
	gdt[num].limit_low = 0;

	gdt[num].access = access;
	gdt[num].granularity = gran;
}

void xddos_gdt_init(void) {
	__asm__ volatile("cli");

	gp.limit = (sizeof(xddos_gdt_entry_t) * 6) - 1;
	gp.base = (uint64_t) &gdt;

	xddos_gdt_set_gate(0, 0, 0);

	// Code segments use Long Mode flag (Bit 5 = 0x20)
	xddos_gdt_set_gate(1, 0x9A, 0x20); // Kernel CS

	// Data/Stack segments use 32-bit Size flag (Bit 6 = 0x40)
	xddos_gdt_set_gate(2, 0x92, 0x40); // Kernel DS
	xddos_gdt_set_gate(3, 0xF2, 0x40); // User DS

	// Code segments use Long Mode flag
	xddos_gdt_set_gate(4, 0xFA, 0x20); // User CS

	// Dummy bootloader segment
	xddos_gdt_set_gate(5, 0x9A, 0x20);

	__asm__ volatile("lgdt %0" : : "m"(gp));

	xddos_gdt_flush();
}