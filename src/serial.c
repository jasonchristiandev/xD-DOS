#include "xddos/asm.h"
#include "xddos/pit.h"
#include <stdint.h>
#define SERIAL_PORT 0x3f8

static uint8_t xddos_serial_initialized = 0;

uint8_t xddos_serial_init() {
	outb(SERIAL_PORT + 1, 0x00); // Disable all interrupts
	outb(SERIAL_PORT + 3, 0x80); // Enable DLAB (set baud rate divisor)
	outb(SERIAL_PORT + 0, 0x03); // Set divisor to 3 (low byte) 38400 baud
	outb(SERIAL_PORT + 1, 0x00); //                  (high byte)
	outb(SERIAL_PORT + 3, 0x03); // 8 bits, no parity, one stop bit
	outb(SERIAL_PORT + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
	outb(SERIAL_PORT + 4, 0x0B); // IRQs enabled, RTS/DSR set
	outb(SERIAL_PORT + 4, 0x1E); // Set in loopback mode, test the serial chip
	outb(SERIAL_PORT + 0, 0xAE); // Test serial chip (send byte 0xAE and check if serial returns same byte)

	// Check if serial is faulty
	if (inb(SERIAL_PORT + 0) != 0xAE) {
		return 0;
	}

	// If serial is not faulty set it in normal operation mode
	outb(SERIAL_PORT + 4, 0x0F);
	xddos_serial_initialized = 1;
	return 1;
}

uint8_t xddos_serial_received() {
	return inb(SERIAL_PORT + 5) & 1;
}

char xddos_serial_read() {
	if (!xddos_serial_initialized) return 0;
	while (xddos_serial_received() == 0);

	return inb(SERIAL_PORT);
}

uint8_t xddos_serial_is_transmit_empty() {
	return inb(SERIAL_PORT + 5) & 0x20;
}

uint8_t xddos_serial_write(char a) {
	if (!xddos_serial_initialized) return 0;

	uint16_t ms_passed = 0;
	while (xddos_serial_is_transmit_empty() == 0) {
		if (ms_passed >= 1000) { // 1000 ms = 1 second
			xddos_serial_initialized = 0;
			return 0;
		}

		SLEEP(1);
		ms_passed++;
	}

	outb(SERIAL_PORT, a);
	return 1;
}

uint8_t xddos_serial_write_text(const char *a) {
	while (*a) {
		if (!xddos_serial_write(*a++)) return 0;
	}
	return 1;
}