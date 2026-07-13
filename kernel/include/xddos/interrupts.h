#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "xddos/requests.h"
#include <stdbool.h>
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
} __attribute__((packed)) interrupts_idtentry_t;

typedef struct {
	uint16_t limit;
	uint64_t base;
} __attribute__((packed)) interrupts_idtr_t;

typedef struct {
	uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
	uint64_t rax, rbx, rcx, rdx, rsi, rdi, rbp;
	uint64_t vector_number;
	uint64_t error_code;
	uint64_t rip, cs, rflags, rsp, ss;
} __attribute__((packed)) interrupts_regstate_t;

typedef enum : uint8_t {
	INTERRUPT_EXCEPTION_TYPE_FAULT = 0,
	INTERRUPT_EXCEPTION_TYPE_TRAP = 1,
	INTERRUPT_EXCEPTION_TYPE_ABORT = 2,
	INTERRUPT_EXCEPTION_TYPE_INTERRUPT = 3,
	INTERRUPT_EXCEPTION_TYPE_RESERVED = 4
} interrupts_exception_type_t;

typedef struct {
	const char *name;
	const char *mnemonic;
	interrupts_exception_type_t type;
	bool has_error_code;
} interrupts_exception_vector_t;

extern const interrupts_exception_vector_t interrupt_exception_vectors[32];
extern const char *interrupt_fault_names[5];

void interrupts_init();
void interrupts_exception_handler(interrupts_regstate_t *state);
void interrupts_set_descriptor(uint8_t vector, void *isr, uint8_t flags);
void interrupts_panic(requests_framebuffer_t *fb, char *message);

#endif // !INTERRUPTS_H