#include "xddos/interrupts.h"
#include "xddos/asm.h"
#include "xddos/graphics.h"
#include "xddos/logging.h"
#include "xddos/requests.h"
#include <stdbool.h>
#define GDT_OFFSET_KERNEL_CODE 0x28 // just following tutorials
#define IDT_ENTRY_NUM 256
#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1
#define ICW1_INIT 0x10
#define ICW1_ICW4 0x01
#define ICW4_8086 0x01

__attribute__((aligned(0x10))) static xddos_interrupts_idtentry_t idt[IDT_ENTRY_NUM];
static xddos_interrupts_idtr_t idtr;
static bool vectors[IDT_ENTRY_NUM];
extern void *isr_stub_table[];
extern xddos_psf_data_t *fallback_font;

void xddos_panic(xddos_framebuffer_t *fb, char *message) {
	(void) fb;
	LOG_ERROR("INTERRUPTS", "Kernel panic! Something went wrong.");
	xddos_graphics_clear(fb, 0x000000);
	xddos_graphics_psf_put_text(fb, fallback_font, "xD-DOS KERNEL PANIC!", 8, 8, 0xFFFFFF, 0x000000);
	xddos_graphics_psf_put_char(fb, fallback_font, '>', 8, 32, 0xFFFFFF, 0x000000);
	xddos_graphics_psf_put_text(fb, fallback_font, message, 28, 32, 0xFFFFFF, 0x000000);
	__asm__ volatile("cli; hlt");
}

void xddos_interrupts_exception_handler(xddos_register_state_t *state) {
	xddos_interrupt_exception_vector_t exception = xddos_interrupt_exception_vectors[state->vector_number];
	LOG_ERROR("INTERRUPTS", "Caught exception in kernel level!\r\n");
	LOG_ERROR("INTERRUPTS", "  Exception: 0x%x", state->vector_number);
	LOG_ERROR("INTERRUPTS", "  Mnemonic: %s", exception.mnemonic);
	LOG_ERROR("INTERRUPTS", "  Type: %s", xddos_interrupt_fault_names[exception.type]);
	LOG_ERROR("INTERRUPTS", "  Name: %s", exception.name);
	LOG_ERROR("INTERRUPTS", "  Error code: 0x%x", state->error_code);
	LOG_ERROR("INTERRUPTS", "  RIP: 0x%llx", state->rip);

	xddos_framebuffers_t *fbs = xddos_request_framebuffers();
	if (fbs == NULL || fbs->count < 1) __asm__ volatile("hlt");
	xddos_panic(fbs->framebuffers[0], "Caught exception in kernel level!\r\nPlease refer to serial console for more information.");
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
	LOG_DEBUG("INTERRUPTS", "Creating interrupt descriptor table...");
	idtr.base = (uintptr_t) &idt[0];
	idtr.limit = (uint16_t) sizeof(xddos_interrupts_idtentry_t) * IDT_ENTRY_NUM - 1;

	for (uint8_t vector = 0; vector < 32; vector++) {
		xddos_interrupts_set_descriptor(vector, isr_stub_table[vector], 0x8E);
		vectors[vector] = true;
	}

	LOG_DEBUG("INTERRUPTS", "Remapping PIC...");

	uint8_t a1 = inb(PIC1_DATA);
	uint8_t a2 = inb(PIC2_DATA);

	outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
	io_wait();
	outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
	io_wait();

	outb(PIC1_DATA, 0x20);
	io_wait();
	outb(PIC2_DATA, 0x28);
	io_wait();

	outb(PIC1_DATA, 4);
	io_wait();
	outb(PIC2_DATA, 2);
	io_wait();

	outb(PIC1_DATA, ICW4_8086);
	io_wait();
	outb(PIC2_DATA, ICW4_8086);
	io_wait();

	outb(PIC1_DATA, a1);
	outb(PIC2_DATA, a2);

	LOG_DEBUG("INTERRUPTS", "Loading interrupt descriptor table...");

	__asm__ volatile("lidt %0" : : "m"(idtr));
	__asm__ volatile("sti");

	LOG_DEBUG("INTERRUPTS", "Done init.");
}