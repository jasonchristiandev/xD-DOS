#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "xddos/requests.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum : uint8_t {
	INTERRUPTS_INIT_OK = 0,
	INTERRUPTS_INIT_MSR_NOT_SUPPORTED = 1,
	INTERRUPTS_INIT_APIC_NOT_SUPPORTED = 2
} interrupts_init_result_t;

typedef struct {
	uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
	uint64_t rax, rbx, rcx, rdx, rsi, rdi, rbp;
	uint64_t vector;
	uint64_t error;
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
	[2] = {"Non-Maskable Interrupt", "-", INTERRUPT_EXCEPTION_TYPE_INTERRUPT, false},
	[3] = {"Breakpoint", "#BP", INTERRUPT_EXCEPTION_TYPE_TRAP, false},
	[4] = {"Overflow", "#OF", INTERRUPT_EXCEPTION_TYPE_TRAP, false},
	[5] = {"Bound Range Exceeded", "#BR", INTERRUPT_EXCEPTION_TYPE_FAULT, false},
	[6] = {"Invalid Opcode", "#UD", INTERRUPT_EXCEPTION_TYPE_FAULT, false},
	[7] = {"Device Not Available", "#NM", INTERRUPT_EXCEPTION_TYPE_FAULT, false},
	[8] = {"Double Fault", "#DF", INTERRUPT_EXCEPTION_TYPE_ABORT, true},
	[9] = {"Coprocessor Segment Overrun", "-", INTERRUPT_EXCEPTION_TYPE_FAULT, false},
	[10] = {"Invalid TSS", "#TS", INTERRUPT_EXCEPTION_TYPE_FAULT, true},
	[11] = {"Segment Not Present", "#NP", INTERRUPT_EXCEPTION_TYPE_FAULT, true},
	[12] = {"Stack-Segment Fault", "#SS", INTERRUPT_EXCEPTION_TYPE_FAULT, true},
	[13] = {"General Protection Fault", "#GP", INTERRUPT_EXCEPTION_TYPE_FAULT, true},
	[14] = {"Page Fault", "#PF", INTERRUPT_EXCEPTION_TYPE_FAULT, true},
	[15] = {"Reserved", "-", INTERRUPT_EXCEPTION_TYPE_RESERVED, false},
	[16] = {"x87 Floating-Point Exception", "#MF", INTERRUPT_EXCEPTION_TYPE_FAULT, false},
	[17] = {"Alignment Check", "#AC", INTERRUPT_EXCEPTION_TYPE_FAULT, true},
	[18] = {"Machine Check", "#MC", INTERRUPT_EXCEPTION_TYPE_ABORT, false},
	[19] = {"SIMD Floating-Point Exception", "#XM/#XF", INTERRUPT_EXCEPTION_TYPE_FAULT, false},
	[20] = {"Virtualization Exception", "#VE", INTERRUPT_EXCEPTION_TYPE_FAULT, false},
	[21] = {"Control Protection Exception", "#CP", INTERRUPT_EXCEPTION_TYPE_FAULT, true},
	[22] = {"Reserved", "-", INTERRUPT_EXCEPTION_TYPE_RESERVED, false},
	[23] = {"Reserved", "-", INTERRUPT_EXCEPTION_TYPE_RESERVED, false},
	[24] = {"Reserved", "-", INTERRUPT_EXCEPTION_TYPE_RESERVED, false},
	[25] = {"Reserved", "-", INTERRUPT_EXCEPTION_TYPE_RESERVED, false},
	[26] = {"Reserved", "-", INTERRUPT_EXCEPTION_TYPE_RESERVED, false},
	[27] = {"Reserved", "-", INTERRUPT_EXCEPTION_TYPE_RESERVED, false},
	[28] = {"Hypervisor Injection Exception", "#HV", INTERRUPT_EXCEPTION_TYPE_FAULT, false},
	[29] = {"VMM Communication Exception", "#VC", INTERRUPT_EXCEPTION_TYPE_FAULT, true},
	[30] = {"Security Exception", "#SX", INTERRUPT_EXCEPTION_TYPE_FAULT, true},
	[31] = {"Reserved", "-", INTERRUPT_EXCEPTION_TYPE_RESERVED, false}};
static const char *interrupt_fault_names[5] = {"FAULT", "TRAP", "ABORT", "INTERRUPT", "RESERVED"};

interrupts_init_result_t interrupts_init();
void interrupts_io_apic_irqwrite(uint8_t irq, uint32_t high, uint32_t low);
void interrupts_panic(char *message);
void interrupts_fail(char *msg1, uint32_t error, char *msg2);

#endif // !INTERRUPTS_H