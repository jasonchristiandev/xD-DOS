#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "xddos/requests.h"
#include <stdbool.h>
#include <stddef.h>
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

typedef enum {
	XDDOS_INTERRUPT_EXCEPTION_TYPE_FAULT = 0,
	XDDOS_INTERRUPT_EXCEPTION_TYPE_TRAP = 1,
	XDDOS_INTERRUPT_EXCEPTION_TYPE_ABORT = 2,
	XDDOS_INTERRUPT_EXCEPTION_TYPE_INTERRUPT = 3,
	XDDOS_INTERRUPT_EXCEPTION_TYPE_RESERVED = 4
} xddos_interrupt_exception_type_t;

typedef struct {
	const char *name;
	const char *mnemonic;
	xddos_interrupt_exception_type_t type;
	bool has_error_code;
} xddos_interrupt_exception_vector_t;

static const xddos_interrupt_exception_vector_t xddos_interrupt_exception_vectors[32] = {
	[0] = {"Division Error", "#DE", XDDOS_INTERRUPT_EXCEPTION_TYPE_FAULT, false},
	[1] = {"Debug", "#DB", XDDOS_INTERRUPT_EXCEPTION_TYPE_FAULT, false},
	[2] = {"Non-maskable Interrupt", "#NMI", XDDOS_INTERRUPT_EXCEPTION_TYPE_INTERRUPT, false},
	[3] = {"Breakpoint", "#BP", XDDOS_INTERRUPT_EXCEPTION_TYPE_TRAP, false},
	[4] = {"Overflow", "#OF", XDDOS_INTERRUPT_EXCEPTION_TYPE_TRAP, false},
	[5] = {"Bound Range Exceeded", "#BR", XDDOS_INTERRUPT_EXCEPTION_TYPE_FAULT, false},
	[6] = {"Invalid Opcode", "#UD", XDDOS_INTERRUPT_EXCEPTION_TYPE_FAULT, false},
	[7] = {"Device Not Available", "#NM", XDDOS_INTERRUPT_EXCEPTION_TYPE_FAULT, false},
	[8] = {"Double Fault", "#DF", XDDOS_INTERRUPT_EXCEPTION_TYPE_ABORT, true},
	[9] = {"Coprocessor Segment Overrun", NULL, XDDOS_INTERRUPT_EXCEPTION_TYPE_FAULT, false},
	[10] = {"Invalid TSS", "#TS", XDDOS_INTERRUPT_EXCEPTION_TYPE_FAULT, true},
	[11] = {"Segment Not Present", "#NP", XDDOS_INTERRUPT_EXCEPTION_TYPE_FAULT, true},
	[12] = {"Stack-Segment Fault", "#SS", XDDOS_INTERRUPT_EXCEPTION_TYPE_FAULT, true},
	[13] = {"General Protection Fault", "#GP", XDDOS_INTERRUPT_EXCEPTION_TYPE_FAULT, true},
	[14] = {"Page Fault", "#PF", XDDOS_INTERRUPT_EXCEPTION_TYPE_FAULT, true},
	[15] = {"Reserved", NULL, XDDOS_INTERRUPT_EXCEPTION_TYPE_RESERVED, false},
	[16] = {"x87 Floating-Point Exception", "#MF", XDDOS_INTERRUPT_EXCEPTION_TYPE_FAULT, false},
	[17] = {"Alignment Check", "#AC", XDDOS_INTERRUPT_EXCEPTION_TYPE_FAULT, true},
	[18] = {"Machine Check", "#MC", XDDOS_INTERRUPT_EXCEPTION_TYPE_ABORT, false},
	[19] = {"SIMD Floating-Point Exception", "#XM/#XF", XDDOS_INTERRUPT_EXCEPTION_TYPE_FAULT, false},
	[20] = {"Virtualization Exception", "#VE", XDDOS_INTERRUPT_EXCEPTION_TYPE_FAULT, false},
	[21] = {"Control Protection Exception", "#CP", XDDOS_INTERRUPT_EXCEPTION_TYPE_FAULT, true},
	[22] = {"Reserved", NULL, XDDOS_INTERRUPT_EXCEPTION_TYPE_RESERVED, false},
	[23] = {"Reserved", NULL, XDDOS_INTERRUPT_EXCEPTION_TYPE_RESERVED, false},
	[24] = {"Reserved", NULL, XDDOS_INTERRUPT_EXCEPTION_TYPE_RESERVED, false},
	[25] = {"Reserved", NULL, XDDOS_INTERRUPT_EXCEPTION_TYPE_RESERVED, false},
	[26] = {"Reserved", NULL, XDDOS_INTERRUPT_EXCEPTION_TYPE_RESERVED, false},
	[27] = {"Reserved", NULL, XDDOS_INTERRUPT_EXCEPTION_TYPE_RESERVED, false},
	[28] = {"Hypervisor Injection Exception", "#HV", XDDOS_INTERRUPT_EXCEPTION_TYPE_FAULT, false},
	[29] = {"VMM Communication Exception", "#VC", XDDOS_INTERRUPT_EXCEPTION_TYPE_FAULT, true},
	[30] = {"Security Exception", "#SX", XDDOS_INTERRUPT_EXCEPTION_TYPE_FAULT, true},
	[31] = {"Reserved", NULL, XDDOS_INTERRUPT_EXCEPTION_TYPE_RESERVED, false}};
static const char *xddos_interrupt_fault_names[5] = {"FAULT", "TRAP", "ABORT", "INTERRUPT", "RESERVED"};

void xddos_interrupts_init();
void xddos_interrupts_exception_handler(xddos_register_state_t *state);
void xddos_interrupts_set_descriptor(uint8_t vector, void *isr, uint8_t flags);
void xddos_panic(xddos_framebuffer_t *fb, char *message);

#endif // !INTERRUPTS_H