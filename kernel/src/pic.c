#include "xddos/asm.h"
#include <stdint.h>

#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1
#define ICW1_INIT 0x10
#define ICW1_ICW4 0x01
#define ICW4_8086 0x01

void xddos_pic_remap(uint8_t offset_a, uint8_t offset_b) {
	// Save masks
	uint8_t a1 = inb(PIC1_DATA);
	uint8_t a2 = inb(PIC2_DATA);

	// Start initialization sequence (in cascade mode)
	outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
	io_wait();
	outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
	io_wait();

	// ICW2: Vector offsets
	outb(PIC1_DATA, offset_a); // Vector offset for PIC1 (e.g., 0x20)
	io_wait();
	outb(PIC2_DATA, offset_b); // Vector offset for PIC2 (e.g., 0x28)
	io_wait();

	// ICW3: Tell Master PIC that there is a slave PIC at IRQ2 (0000 0100b)
	outb(PIC1_DATA, 4);
	io_wait();
	// Tell Slave PIC its cascade identity (0000 0010b)
	outb(PIC2_DATA, 2);
	io_wait();

	// ICW4: Set mode to 8086/88
	outb(PIC1_DATA, ICW4_8086);
	io_wait();
	outb(PIC2_DATA, ICW4_8086);
	io_wait();

	// Restore masks
	outb(PIC1_DATA, a1);
	outb(PIC2_DATA, a2);
}