#include "xD-DOS/asm.h"
#include "xD-DOS/string.h" // IWYU pragma: keep
#include <stdbool.h>
#include <stddef.h>
#define PORT 0x3f8

bool serial_init() {
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
		return 1;
	}

	// If serial is not faulty set it in normal operation mode
	outb(PORT + 4, 0x0F);
	return 0;
}

bool serial_received() {
	return inb(PORT + 5) & 1;
}

char serial_read() {
	while (serial_received() == 0);

	return inb(PORT);
}

bool serial_is_transmit_empty() {
	return (inb(PORT + 5) & 0x20) >> 5;
}

void serial_write(char a) {
	while (serial_is_transmit_empty() == 0);

	outb(PORT, a);
}

void serial_write_text(char *a) {
	size_t n = strlen(a);
	for (size_t i = 0; i < n; i++) {
		serial_write(a[i]);
	}
}