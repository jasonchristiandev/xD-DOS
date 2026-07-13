#include "xddos/interrupts.h"
#include "xddos/asm.h"
#include "xddos/graphics.h"
#include "xddos/kstdio.h"
#include "xddos/logging.h"
#include "xddos/main.h"
#include "xddos/pit.h"
#include "xddos/requests.h"
#include <stdbool.h>

#define GDT_OFFSET_KERNEL_CODE 0x08
#define IDT_ENTRY_NUM 256
#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1
#define ICW1_INIT 0x10
#define ICW1_ICW4 0x01
#define ICW4_8086 0x01

__attribute__((aligned(0x10))) interrupts_idt_entry_t idt[IDT_ENTRY_NUM];
interrupts_idt_pointer_t idtr;
bool vectors[IDT_ENTRY_NUM];
extern void *isr_stub_table[];

const uint32_t qrcode[29] = {
	0x1fc7547f, 0x104cc941, 0x1743525d, 0x1750885d,
	0x175adf5d, 0x10401941, 0x1fd5557f, 0x00041400,
	0x18e96818, 0x1f2a66b6, 0x02646c10, 0x01981288,
	0x0a7a7161, 0x191abf73, 0x1450659c, 0x0ca72635,
	0x05dc4aac, 0x18223277, 0x1a4aa619, 0x16030540,
	0x14f46bf7, 0x001eed18, 0x1fdcf75c, 0x10514311,
	0x174d93fb, 0x1746738f, 0x174caffe, 0x1052006d,
	0x1fdbba74};

void interrupts_panic(requests_framebuffer_t *fb, char *message) {
	__asm__ __volatile__("cli");
	if (fallback_font == NULL) {
		__asm__ __volatile__("hlt");
	}
	graphics_clear(fb, 0x000000);
	pit_sleep_ms(100);

	graphics_psf_put_text(fb, fallback_font, ":( xD-DOS KERNEL PANIC!", 8, 8, 0xFFFFFF, 0x000000);
	graphics_psf_put_char(fb, fallback_font, '>', 8, 32, 0xFFFFFF, 0x000000);
	graphics_psf_put_text(fb, fallback_font, message, 28, 32, 0xFFFFFF, 0x000000);
	char *msg1 = "Unexpected kernel exception? Please report this issue at\r\nhttps://github.com/jasonchristiandev/xD-DOS/issues";
	char *msg2 = "or through the QR code below.";
	LOG_ERROR("INTERRUPTS", msg1);

	uint32_t y = 56;
	for (int i = 0; message[i] != '\0'; i++) {
		if (message[i] == '\n') y += 16;
	}

	graphics_psf_put_text(fb, fallback_font, msg1, 28, y, 0xFFFFFF, 0x000000);
	graphics_psf_put_text(fb, fallback_font, msg2, 28, y + 32, 0xFFFFFF, 0x000000);
	y += 52;

	const uint8_t pixel_size = 4;
	graphics_rect(fb, 28, y, pixel_size * 33, pixel_size * 33, 0xFFFFFF);

	for (int yi = 0; yi < 29; yi++) {
		for (int xi = 0; xi < 29; xi++) {
			if ((qrcode[yi] >> (28 - xi)) & 1) {
				uint32_t sx = 28 + (xi + 2) * pixel_size;
				uint32_t sy = y + (yi + 2) * pixel_size;

				graphics_rect(fb, sx, sy, pixel_size, pixel_size, 0x000000);
			}
		}
	}

	y += 33 * pixel_size + 8;

	graphics_psf_put_text(fb, fallback_font, "Press [ENTER] to reboot.\r\n      [F1] to power off.", 28, y, 0xFFFFFF, 0x000000);
	pit_sleep_ms(1000);
	for (;;) {
		if ((inb(0x64) & 0x01) != 0) {
			uint8_t sc = inb(0x60);
			if (sc == 59) {			 // f1
				outw(0x604, 0x2000); // qemu only (for now)
				__asm__ __volatile__("hlt");
			} else if (sc == 28) { // enter
				outb(0x64, 0xFE);
				__asm__ __volatile__("hlt");
			}
		}
	}
}

char msg[2048];

void interrupts_exception_handler(interrupts_register_state_t *state) {
	if (state->vector_number >= 32) {
		if (state->vector_number >= 40) outb(PIC2_COMMAND, 0x20);
		outb(PIC1_COMMAND, 0x20);
	}

	uint64_t cr2;
	__asm__ volatile("mov %%cr2, %0" : "=r"(cr2));
	interrupts_exception_vector_t exception = interrupt_exception_vectors[state->vector_number];
	kstdio_snprintf(msg, 256, "Caught exception in kernel level!\r\n"
							  "  Exception: 0x%x\r\n"
							  "  Mnemonic: %s\r\n"
							  "  Type: %s\r\n"
							  "  Name: %s\r\n"
							  "  Error Code: 0x%x\r\n"
							  "  RIP: 0x%llx\r\n"
							  "  CR2: 0x%llx",
					state->vector_number,
					exception.mnemonic,
					interrupt_fault_names[exception.type],
					exception.name,
					state->error_code,
					state->rip,
					cr2);
	LOG_ERROR("INTERRUPTS", msg);

	requests_framebuffers_t *fbs = request_framebuffers();
	if (fbs == NULL || fbs->count < 1) __asm__ volatile("hlt");
	requests_framebuffer_t *fb = fbs->framebuffers[0];
	interrupts_panic(fb, msg);
}

void interrupts_set_descriptor(uint8_t vector, void *isr, uint8_t flags) {
	interrupts_idt_entry_t *descriptor = &idt[vector];

	descriptor->isr_low = (uint64_t) isr & 0xFFFF;
	descriptor->selector = GDT_OFFSET_KERNEL_CODE;
	descriptor->ist = 0;
	descriptor->attr = flags;
	descriptor->isr_mid = ((uint64_t) isr >> 16) & 0xFFFF;
	descriptor->isr_high = ((uint64_t) isr >> 32) & 0xFFFFFFFF;
	descriptor->reserved = 0;
}

void interrupts_init() {
	LOG_DEBUG("INTERRUPTS", "Creating interrupt descriptor table...");
	idtr.base = (uintptr_t) &idt[0];
	idtr.limit = (uint16_t) sizeof(interrupts_idt_entry_t) * IDT_ENTRY_NUM - 1;

	for (uint8_t vector = 0; vector < 48; vector++) {
		interrupts_set_descriptor(vector, isr_stub_table[vector], 0x8E);
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