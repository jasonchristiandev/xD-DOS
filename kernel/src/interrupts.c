#include "xddos/interrupts.h"
#include "xddos/asm.h"
#include "xddos/cpuid.h"
#include "xddos/graphics.h"
#include "xddos/idt.h"
#include "xddos/kstdio.h"
#include "xddos/logging.h"
#include "xddos/pit.h"
#include <stdbool.h>

#define PIC1_CMD 0x20
#define PIC1_DATA 0x21
#define PIC2_CMD 0xA0
#define PIC2_DATA 0xA1

__attribute__((aligned(0x10))) static idt_entry_t idt[256];
static idt_pointer_t idt_ptr;
extern void *isr_stub_table[];
uint64_t apic_base;
uint64_t lapic_base;
uint64_t io_apic_base;

void set_descriptor(uint8_t vector, void *isr, uint8_t flags) {
	idt_entry_t *descriptor = &idt[vector];

	descriptor->isr_low = (uint64_t) isr;
	descriptor->selector = 0x08;
	descriptor->ist = 0;
	descriptor->flags = flags;
	descriptor->isr_mid = (uint64_t) isr >> 16;
	descriptor->isr_high = (uint64_t) isr >> 32;
	descriptor->reserved = 0;
}

uint32_t lapic_read(uint32_t reg) {
	volatile uint32_t *addr = (volatile uint32_t *) (lapic_base + reg);
	return *addr;
}

void lapic_write(uint32_t reg, uint32_t val) {
	volatile uint32_t *addr = (volatile uint32_t *) (lapic_base + reg);
	*addr = val;
}

void io_apic_write(uint8_t reg, uint32_t value) {
	volatile uint32_t *regsel = (volatile uint32_t *) io_apic_base;
	volatile uint32_t *iowin = (volatile uint32_t *) (io_apic_base + 16);

	*regsel = reg;
	*iowin = value;
}

void interrupts_io_apic_irqwrite(uint8_t irq, uint32_t high, uint32_t low) {
	uint8_t reg = 0x10 + (irq_to_gsi[irq] << 1);
	io_apic_write(reg + 1, high);
	io_apic_write(reg, low);
}

interrupts_init_result_t interrupts_init() {
	// check features
	if (!cpuid_msr()) return INTERRUPTS_INIT_MSR_NOT_SUPPORTED;
	if (!cpuid_apic()) return INTERRUPTS_INIT_APIC_NOT_SUPPORTED;

	LOG_DEBUG("INTERRUPTS", "Setting descriptors...");
	idt_ptr.base = (uint64_t) &idt;
	idt_ptr.limit = sizeof(idt_entry_t) * 256 - 1;

	for (uint16_t i = 0; i < 256; i++) {
		// 1    present
		// 00   dpl
		// 0
		// 1110 interrupt gate
		set_descriptor(i, isr_stub_table[i], 0b10001110);
	}

	LOG_DEBUG("INTERRUPTS", "Enabling APIC...");

	// disable 8259 pic
	outb(PIC1_DATA, 0xFF);
	outb(PIC2_DATA, 0xFF);

	apic_base = rdmsr(0x1B);
	lapic_base = apic_base & 0xFFFFFFFFFFFFF000;

	// enable lapic
	// set spurious to 0xFF
	lapic_write(0xF0, lapic_read(0xF0) | 0x100 | 0xFF);

	io_apic_base = acpi_io_apic_entry->io_apic_ptr;

	interrupts_io_apic_irqwrite(1, 0, 33); // unmask keyboard

	__asm__ __volatile__("lidt %0" : : "m"(idt_ptr));
	__asm__ __volatile__("sti");

	inb(0x60);

	LOG_DEBUG("INTERRUPTS", "Done init.");

	return INTERRUPTS_INIT_OK;
}

void interrupts_eoi() {
	lapic_write(0xB0, 0);
}

char msg[2048];
void interrupts_handler(interrupts_regstate_t *state) {
	if (state->vector >= 32) {
		// skip spurious interrupts
		if (state->vector == 0xFF) return;

		// ps/2 keyboard
		if (state->vector == 33) {
			uint8_t sc = inb(0x60);
			kstdio_snprintf(msg, 2048, "sc 0x%x", sc);
			LOG_INFO("INTERRUPTS", msg);
		}

		interrupts_eoi();
		return;
	}

	uint64_t cr2;
	__asm__ __volatile__("mov %%cr2, %0" : "=r"(cr2));
	interrupts_exception_vector_t exception = interrupt_exception_vectors[state->vector];
	kstdio_snprintf(msg, 2048, "Caught exception in kernel level!\r\n"
							   "  Exception: 0x%x\r\n"
							   "  Mnemonic: %s\r\n"
							   "  Type: %s\r\n"
							   "  Name: %s\r\n"
							   "  Error Code: 0x%x\r\n"
							   "  RIP: 0x%llx\r\n"
							   "  CR2: 0x%llx",
					state->vector,
					exception.mnemonic,
					interrupt_fault_names[exception.type],
					exception.name,
					state->error,
					state->rip,
					cr2);
	LOG_ERROR("INTERRUPTS", msg);

	interrupts_panic(msg);
}

const uint32_t qrcode[29] = {
	0x1fc7547f, 0x104cc941, 0x1743525d, 0x1750885d,
	0x175adf5d, 0x10401941, 0x1fd5557f, 0x00041400,
	0x18e96818, 0x1f2a66b6, 0x02646c10, 0x01981288,
	0x0a7a7161, 0x191abf73, 0x1450659c, 0x0ca72635,
	0x05dc4aac, 0x18223277, 0x1a4aa619, 0x16030540,
	0x14f46bf7, 0x001eed18, 0x1fdcf75c, 0x10514311,
	0x174d93fb, 0x1746738f, 0x174caffe, 0x1052006d,
	0x1fdbba74};

void interrupts_panic(char *message) {
	__asm__ __volatile__("cli");
	LOG_ERROR("INTERRUPTS", "Kernel panic!");
	if (fb != NULL) {
		graphics_clear(0x000000);
	}
	pit_sleep_ms(200);

	if (fb != NULL && fallback_font != NULL) {
		graphics_psf_put_text(fallback_font, ":( xD-DOS KERNEL PANIC!", 8, 8, 0xFFFFFF, 0x000000);
		graphics_psf_put_char(fallback_font, '>', 8, 32, 0xFFFFFF, 0x000000);
		graphics_psf_put_text(fallback_font, message, 28, 32, 0xFFFFFF, 0x000000);
	}

	char *msg1 = "Unexpected kernel exception? Please report this issue at\r\nhttps://github.com/jasonchristiandev/xD-DOS/issues";
	char *msg2 = "or through the QR code below.";
	LOG_ERROR("INTERRUPTS", msg1);

	uint32_t y = 56;
	for (int i = 0; message[i] != '\0'; i++) {
		if (message[i] == '\n') y += 16;
	}

	if (fb != NULL && fallback_font != NULL) {
		graphics_psf_put_text(fallback_font, msg1, 28, y, 0xFFFFFF, 0x000000);
		graphics_psf_put_text(fallback_font, msg2, 28, y + 32, 0xFFFFFF, 0x000000);
	}

	y += 52;

	const uint8_t pixel_size = 4;
	if (fb != NULL) graphics_rect(28, y, pixel_size * 33, pixel_size * 33, 0xFFFFFF);

	for (int yi = 0; yi < 29; yi++) {
		for (int xi = 0; xi < 29; xi++) {
			if ((qrcode[yi] >> (28 - xi)) & 1) {
				uint32_t sx = 28 + (xi + 2) * pixel_size;
				uint32_t sy = y + (yi + 2) * pixel_size;

				if (fb != NULL) graphics_rect(sx, sy, pixel_size, pixel_size, 0x000000);
			}
		}
	}

	y += 33 * pixel_size + 8;

	hlt();
}

void interrupts_fail(char *msg1, uint32_t error, char *msg2) {
	kstdio_snprintf(msg, 2048, "%s 0x%x (%s)", msg1, error, msg2);
	LOG_ERROR("INTERRUPTS", msg);
	kstdio_snprintf(msg, 2048, "%s\r\n0x%x (%s)", msg1, error, msg2);
	interrupts_panic(msg);
}
