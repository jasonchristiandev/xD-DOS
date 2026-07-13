#include "xddos/interrupts.h"
#include "xddos/acpi.h"
#include "xddos/asm.h"
#include "xddos/graphics.h"
#include "xddos/kstdio.h"
#include "xddos/logging.h"
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

__attribute__((aligned(0x10))) static interrupts_idtentry_t idt[IDT_ENTRY_NUM];
static interrupts_idtr_t idtr;
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
const interrupts_exception_vector_t interrupt_exception_vectors[32] = {
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
const char *interrupt_fault_names[5] = {"FAULT", "TRAP", "ABORT", "INTERRUPT", "RESERVED"};
extern void *isr_stub_table[];
extern psf_data_t *fallback_font;
extern acpi_fadt_t *global_fadt;

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
	char *msg1 = "Unexpected kernel exception? Please report this issue at:\r\nhttps://github.com/jasonchristiandev/xD-DOS/issues";
	char *msg2 = "or through the below QR code:";
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
			if ((qrcode[yi] >> xi) & 1) {
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

void interrupts_exception_handler(interrupts_regstate_t *state) {
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
	interrupts_idtentry_t *descriptor = &idt[vector];

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
	idtr.limit = (uint16_t) sizeof(interrupts_idtentry_t) * IDT_ENTRY_NUM - 1;

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