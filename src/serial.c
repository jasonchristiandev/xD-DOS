#include "xddos/asm.h"
#include "xddos/pit.h"
#include <stddef.h>
#define PORT 0x3f8

static uint8_t serial_initialized = 0;

uint8_t serial_init() {
	outb(PORT + 1, 0x00); // Disable all interrupts
	outb(PORT + 3, 0x80); // Enable DLAB (set baud rate divisor)
	outb(PORT + 0, 0x03); // Set divisor to 3 (low byte) 38400 baud
	outb(PORT + 1, 0x00); //                  (high byte)
	outb(PORT + 3, 0x03); // 8 bits, no parity, one stop bit
	outb(PORT + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
	outb(PORT + 4, 0x0B); // IRQs enabled, RTS/DSR set
	outb(PORT + 4, 0x1E); // Set in loopback mode, test the serial chip
	outb(PORT + 0, 0xAE); // Test serial chip (send byte 0xAE and check if serial returns same byte)

	// Check if serial is faulty
	if (inb(PORT + 0) != 0xAE) {
		return 0;
	}

	// If serial is not faulty set it in normal operation mode
	outb(PORT + 4, 0x0F);
	serial_initialized = 1;
	return 1;
}

uint8_t serial_received() {
	return inb(PORT + 5) & 1;
}

char serial_read() {
	if (!serial_initialized) return 0;
	while (serial_received() == 0);

	return inb(PORT);
}

uint8_t serial_is_transmit_empty() {
	return inb(PORT + 5) & 0x20;
}

uint8_t serial_write(char a) {
	if (!serial_initialized) return 0;

	int ms_passed = 0;
	while (serial_is_transmit_empty() == 0) {
		if (ms_passed >= 1000) { // 1000 ms = 1 second
			serial_initialized = 0;
			return 0;
		}

		SLEEP(1);
		ms_passed++;
	}

	outb(PORT, a);
	return 1;
}

uint8_t serial_write_text(const char *a) {
	while (*a) {
		if (!serial_write(*a++)) return 0;
	}
	return 1;
}