#include "xddos/interrupts.h"
#include "xddos/logging.h"
#include "xddos/pic.h"
#include <stdbool.h>
#define GDT_OFFSET_KERNEL_CODE 0x28 // just following tutorials
#define IDT_ENTRY_NUM 256

__attribute__((aligned(0x10))) static xddos_interrupts_idtentry_t idt[IDT_ENTRY_NUM];
static xddos_interrupts_idtr_t idtr;
static bool vectors[IDT_ENTRY_NUM];
extern void *isr_stub_table[];

void xddos_interrupts_exception_handler(xddos_register_state_t *state) {
	if (state->vector_number < 31) {
		int x = 1 / 0;
		(void) x;
	}
	xddos_interrupt_exception_vector_t exception = xddos_interrupt_exception_vectors[state->vector_number];
	LOG_ERROR("INT", "Caught exception in kernel level!");
	LOG_ERROR("INT", "  Exception: 0x%x", state->vector_number);
	LOG_ERROR("INT", "  Mnemonic: %s", exception.mnemonic);
	LOG_ERROR("INT", "  Type: %s", xddos_interrupt_fault_names[exception.type]);
	LOG_ERROR("INT", "  Name: %s", exception.name);
	LOG_ERROR("INT", "  Error code: 0x%x", state->error_code);
	LOG_ERROR("INT", "  RIP: 0x%llx", state->rip);

	__asm__ volatile("cli; hlt");
}

void xddos_interrupts_set_descriptor(uint8_t vector, void *isr, uint8_t flags) {
	xddos_interrupts_idtentry_t *descriptor = &idt[vector];

	descriptor->isr_low = (uint64_t) isr & 0xFFFF;
	descriptor->selector = GDT_OFFSET_KERNEL_CODE;
	descriptor->ist = 0;
	descriptor->attr = flags;
	descriptor->isr_mid = ((uint64_t) isr >> 16) & 0xFFFF;
	descriptor->isr_high = ((uint64_t) isr >> 32) & 0xFFFFFFFF;
	descriptor->reserved = 0;
}

#define GDT_OFFSET_KERNEL_CODE 0x28

void xddos_interrupts_init() {
	idtr.base = (uintptr_t) &idt[0];
	idtr.limit = (uint16_t) sizeof(xddos_interrupts_idtentry_t) * IDT_ENTRY_NUM - 1;

	for (uint8_t vector = 0; vector < 32; vector++) {
		xddos_interrupts_set_descriptor(vector, isr_stub_table[vector], 0x8E);
		vectors[vector] = true;
	}

	xddos_pic_remap(0x20, 0x28);

	__asm__ volatile("lidt %0" : : "m"(idtr));
	__asm__ volatile("sti");
}