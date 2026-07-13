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

static const interrupts_exception_vector_t interrupt_exception_vectors[32] = {
	[0] = {"Division Error", "#DE", INTERRUPT_EXCEPTION_TYPE_FAULT, false},
	[1] = {"Debug", "#DB", INTERRUPT_EXCEPTION_TYPE_FAULT, false},
	[2] = {"Non-maskable Interrupt", "#NMI", INTERRUPT_EXCEPTION_TYPE_INTERRUPT, false},
	[3] = {"Breakpoint", "#BP", INTERRUPT_EXCEPTION_TYPE_TRAP, false},
	[4] = {"Overflow", "#OF", INTERRUPT_EXCEPTION_TYPE_TRAP, false},
	[5] = {"Bound Range Exceeded", "#BR", INTERRUPT_EXCEPTION_TYPE_FAULT, false},
	[6] = {"Invalid Opcode", "#UD", INTERRUPT_EXCEPTION_TYPE_FAULT, false},
	[7] = {"Device Not Available", "#NM", INTERRUPT_EXCEPTION_TYPE_FAULT, false},
	[8] = {"Double Fault", "#DF", INTERRUPT_EXCEPTION_TYPE_ABORT, true},
	[9] = {"Coprocessor Segment Overrun", NULL, INTERRUPT_EXCEPTION_TYPE_FAULT, false},
	[10] = {"Invalid TSS", "#TS", INTERRUPT_EXCEPTION_TYPE_FAULT, true},
	[11] = {"Segment Not Present", "#NP", INTERRUPT_EXCEPTION_TYPE_FAULT, true},
	[12] = {"Stack-Segment Fault", "#SS", INTERRUPT_EXCEPTION_TYPE_FAULT, true},
	[13] = {"General Protection Fault", "#GP", INTERRUPT_EXCEPTION_TYPE_FAULT, true},
	[14] = {"Page Fault", "#PF", INTERRUPT_EXCEPTION_TYPE_FAULT, true},
	[15] = {"Reserved", NULL, INTERRUPT_EXCEPTION_TYPE_RESERVED, false},
	[16] = {"x87 Floating-Point Exception", "#MF", INTERRUPT_EXCEPTION_TYPE_FAULT, false},
	[17] = {"Alignment Check", "#AC", INTERRUPT_EXCEPTION_TYPE_FAULT, true},
	[18] = {"Machine Check", "#MC", INTERRUPT_EXCEPTION_TYPE_ABORT, false},
	[19] = {"SIMD Floating-Point Exception", "#XM/#XF", INTERRUPT_EXCEPTION_TYPE_FAULT, false},
	[20] = {"Virtualization Exception", "#VE", INTERRUPT_EXCEPTION_TYPE_FAULT, false},
	[21] = {"Control Protection Exception", "#CP", INTERRUPT_EXCEPTION_TYPE_FAULT, true},
	[22] = {"Reserved", NULL, INTERRUPT_EXCEPTION_TYPE_RESERVED, false},
	[23] = {"Reserved", NULL, INTERRUPT_EXCEPTION_TYPE_RESERVED, false},
	[24] = {"Reserved", NULL, INTERRUPT_EXCEPTION_TYPE_RESERVED, false},
	[25] = {"Reserved", NULL, INTERRUPT_EXCEPTION_TYPE_RESERVED, false},
	[26] = {"Reserved", NULL, INTERRUPT_EXCEPTION_TYPE_RESERVED, false},
	[27] = {"Reserved", NULL, INTERRUPT_EXCEPTION_TYPE_RESERVED, false},
	[28] = {"Hypervisor Injection Exception", "#HV", INTERRUPT_EXCEPTION_TYPE_FAULT, false},
	[29] = {"VMM Communication Exception", "#VC", INTERRUPT_EXCEPTION_TYPE_FAULT, true},
	[30] = {"Security Exception", "#SX", INTERRUPT_EXCEPTION_TYPE_FAULT, true},
	[31] = {"Reserved", NULL, INTERRUPT_EXCEPTION_TYPE_RESERVED, false}};
static const char *interrupt_fault_names[5] = {"FAULT", "TRAP", "ABORT", "INTERRUPT", "RESERVED"};

extern const interrupts_exception_vector_t interrupt_exception_vectors[32];
extern const char *interrupt_fault_names[5];

void interrupts_init();
void interrupts_exception_handler(interrupts_regstate_t *state);
void interrupts_set_descriptor(uint8_t vector, void *isr, uint8_t flags);
void interrupts_panic(requests_framebuffer_t *fb, char *message);

#endif // !INTERRUPTS_H