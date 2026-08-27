#ifndef __XDDOS_GDT_H
#define __XDDOS_GDT_H

#include <stdint.h>

typedef struct {
	uint16_t limit_low;	 // no use
	uint16_t base_low;	 // no use
	uint8_t base_middle; // no use
	uint8_t access;
	uint8_t granularity;
	uint8_t base_high; // no use
} __attribute__((packed)) gdt_entry_t;

typedef struct {
	uint16_t limit;
	uint64_t base;
} __attribute__((packed)) gdt_pointer_t;

extern gdt_entry_t gdt[6];
extern gdt_pointer_t gdt_ptr;

void gdt_set_gate(int num, uint8_t access, uint8_t granularity);
void gdt_init();

#endif // !__XDDOS_GDT_H