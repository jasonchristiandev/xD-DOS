#include "xddos/interrupts.h"
#include "xddos/acpi.h"
#include "xddos/asm.h"
#include "xddos/graphics.h"
#include "xddos/logging.h"
#include "xddos/pit.h"
#include "xddos/requests.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
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
static const uint32_t qrcode[29] = {
	0b11111110001110101010001111111,
	0b10000010011001100100101000001,
	0b10111010000110101001001011101,
	0b10111010100001000100001011101,
	0b10111010110101101111101011101,
	0b10000010000000001100101000001,
	0b11111110101010101010101111111,
	0b00000000001000001010000000000,
	0b11000111010010110100000011000,
	0b11111001010100110011010110110,
	0b00010011001000110110000010000,
	0b00001100110000001001010001000,
	0b01010011110100111000101100001,
	0b11001000110101011111101110011,
	0b10100010100000110010110011100,
	0b01100101001110010011000110101,
	0b00101110111000100101010101100,
	0b11000001000100011001001110111,
	0b11010010010101010011000011001,
	0b10110000000110000010101000000,
	0b10100111101000110101111110111,
	0b00000000111101110110100011000,
	0b11111110111001111011101011100,
	0b10000010100010100001100010001,
	0b10111010011011001001111111011,
	0b10111010001100111001110001111,
	0b10111010011001010111111111110,
	0b10000010100100000000001101101,
	0b11111110110111011101001110100};
extern void *isr_stub_table[];
extern xddos_psf_data_t *fallback_font;
extern xddos_acpi_fadt_t *global_fadt;

void xddos_panic(xddos_framebuffer_t *fb, char *message) {
	__asm__ __volatile__("cli");
	if (fallback_font == NULL) {
		__asm__ __volatile__("hlt");
	}
	xddos_graphics_clear(fb, 0x000000);
	xddos_pit_sleep_ms(100);

	(void) fb;
	xddos_graphics_psf_put_text(fb, fallback_font, ":( xD-DOS KERNEL PANIC!", 8, 8, 0xFFFFFF, 0x000000);
	xddos_graphics_psf_put_char(fb, fallback_font, '>', 8, 32, 0xFFFFFF, 0x000000);
	xddos_graphics_psf_put_text(fb, fallback_font, message, 28, 32, 0xFFFFFF, 0x000000);
	char *msg1 = "Unexpected kernel exception? Please report this issue at:\r\nhttps://github.com/jasonchristiandev/xD-DOS/issues";
	char *msg2 = "or through the below QR code:";
	LOG_ERROR("INTERRUPTS", msg1);

	uint32_t y = 56;
	for (int i = 0; message[i] != '\0'; i++) {
		if (message[i] == '\n') y += 16;
	}

	xddos_graphics_psf_put_text(fb, fallback_font, msg1, 28, y, 0xFFFFFF, 0x000000);
	xddos_graphics_psf_put_text(fb, fallback_font, msg2, 28, y + 32, 0xFFFFFF, 0x000000);
	y += 52;

	const uint8_t pixel_size = 4;
	xddos_graphics_rect(fb, 28, y, pixel_size * 33, pixel_size * 33, 0xFFFFFF);

	for (int yi = 0; yi < 29; yi++) {
		for (int xi = 0; xi < 29; xi++) {
			if ((qrcode[yi] >> xi) & 1) {
				uint32_t sx = 28 + (xi + 2) * pixel_size;
				uint32_t sy = y + (yi + 2) * pixel_size;

				xddos_graphics_rect(fb, sx, sy, pixel_size, pixel_size, 0x000000);
			}
		}
	}

	y += 33 * pixel_size + 8;

	xddos_graphics_psf_put_text(fb, fallback_font, "Press [ENTER] to reboot.\r\n      [F1] to power off.", 28, y, 0xFFFFFF, 0x000000);
	xddos_pit_sleep_ms(1000);
	for (;;) {
		if ((inb(0x64) & 0x01) != 0) {
			uint8_t sc = inb(0x60);
			if (sc == 59) { // f1
				outw(0x604, 0x2000); // qemu only (for now)
				__asm__ __volatile__("hlt");
			} else if (sc == 28) { // enter
				outb(0x64, 0xFE);
				__asm__ __volatile__("hlt");
			}
		}
	}
}

void xddos_interrupts_exception_handler(xddos_register_state_t *state) {
	xddos_interrupt_exception_vector_t exception = xddos_interrupt_exception_vectors[state->vector_number];
	char *msg = malloc(256);
	snprintf(msg, 256, "Caught exception in kernel level!\r\n  Exception: 0x%x\r\n  Mnemonic: %s\r\n  Type: %s\r\n  Name: %s\r\n  Error Code: 0x%x\r\n  RIP: 0x%llx", state->vector_number, exception.mnemonic, xddos_interrupt_fault_names[exception.type], exception.name, state->error_code, state->rip);
	LOG_ERROR("INTERRUPTS", msg);

	xddos_framebuffers_t *fbs = xddos_request_framebuffers();
	if (fbs == NULL || fbs->count < 1) __asm__ volatile("hlt");
	xddos_framebuffer_t *fb = fbs->framebuffers[0];
	xddos_panic(fb, msg);
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

	for (uint8_t vector = 0; vector < 48; vector++) {
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