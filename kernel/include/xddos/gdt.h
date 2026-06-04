#ifndef GDT_H
#define GDT_H

#include <stdint.h>

typedef struct {
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t base_middle;
	uint8_t access;
	uint8_t granularity;
	uint8_t base_high;
} __attribute__((packed)) xddos_gdt_entry_t;

typedef struct {
	uint16_t limit;
	uint64_t base;
} __attribute__((packed)) xddos_gdt_ptr_t;

extern xddos_gdt_entry_t gdt[6];
extern xddos_gdt_ptr_t gp;

void xddos_gdt_set_gate(int num, uint8_t access, uint8_t gran);
void xddos_gdt_init();

#endif // !GDT_H