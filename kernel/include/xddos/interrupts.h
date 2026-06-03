#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

// https://wiki.osdev.org/Interrupts_Tutorial
typedef struct {
	uint16_t isr_low;  // offset bits 0..15
	uint16_t selector; // a code segment selector in GDT or LDT
	uint8_t ist;	   // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.
	uint8_t attr;	   // gate type, dpl, and p fields
	uint16_t isr_mid;  // offset bits 16..31
	uint32_t isr_high; // offset bits 32..63
	uint32_t reserved; //
} __attribute__((packed)) xddos_interrupts_idtentry_t;

typedef struct {
	uint16_t limit;
	uint64_t base;
} __attribute__((packed)) xddos_interrupts_idtr_t;

typedef struct {
	uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
	uint64_t rax, rbx, rcx, rdx, rsi, rdi, rbp;
	uint64_t vector_number;
	uint64_t error_code;
	uint64_t rip, cs, rflags, rsp, ss;
} __attribute__((packed)) xddos_register_state_t;

void xddos_interrupts_init(void);
void xddos_interrupts_exception_handler(xddos_register_state_t *state);
void xddos_interrupts_set_descriptor(uint8_t vector, void *isr, uint8_t flags);

#endif // !INTERRUPTS_H