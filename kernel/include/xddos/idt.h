#ifndef __XDDOS_IDT_H
#define __XDDOS_IDT_H

#include <stdint.h>

typedef struct {
	uint16_t isr_low;
	uint16_t selector;
	uint8_t ist;
	uint8_t flags;
	uint16_t isr_mid;
	uint32_t isr_high;
	uint32_t reserved;
} __attribute__((packed)) idt_entry_t;

typedef struct {
	uint16_t limit;
	uint64_t base;
} __attribute__((packed)) idt_pointer_t;

#endif // !__XDDOS_IDT_H