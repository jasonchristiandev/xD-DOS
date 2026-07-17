#include "xddos/asm.h"
#include "xddos/interrupts.h"
#include "xddos/pit.h"
#include "xddos/ps2.h"

#define PS2_DATA 0x60
#define PS2_CMD 0x64
#define PS2_STATUS 0x64

void ps2_wait_write() {
	uint8_t timeout = 100;
	while ((inb(PS2_STATUS) & 2) != 0) {
		pit_sleep_ms(1);
		timeout--;
		if (timeout == 0) return;
	}
}

void ps2_wait_read() {
	uint8_t timeout = 100;
	while ((inb(PS2_STATUS) & 1) == 0) {
		pit_sleep_ms(1);
		timeout--;
		if (timeout == 0) return;
	}
}

void ps2_mouse_write(uint8_t value) {
	ps2_wait_write();
	outb(PS2_CMD, 0xD4);
	ps2_wait_write();
	outb(PS2_DATA, value);
}

uint8_t ps2_mouse_read() {
	ps2_wait_read();
	return inb(PS2_DATA);
}

void ps2_mouse_init(uint8_t lapic) {
	interrupts_io_apic_irqwrite(12, ((uint32_t) lapic) << 24, 44);

	ps2_wait_write();
	outb(PS2_CMD, 0xA8);

	ps2_wait_write();
	outb(PS2_CMD, 0x20);
	ps2_wait_read();
	uint8_t config = inb(PS2_DATA);

	config |= 0x02;
	config &= ~0x20;

	ps2_wait_write();
	outb(PS2_CMD, 0x60);
	ps2_wait_write();
	outb(PS2_DATA, config);

	ps2_mouse_write(0xF4);
	ps2_mouse_read();
}