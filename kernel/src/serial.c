#include "xddos/serial.h"
#include "xddos/asm.h"
#include "xddos/pit.h"

#define COM1 0x3F8

static bool initialized = false;

bool xddos_serial_init() {
	// - disable interrupts
	// - set dlab to 1, data to 7, parity to none, stop to 1, break to 0?
	// - set baud rate (115200/3 = 38400)
	// - clean and turn on fifo???
	// - set dlab to 0

	outb(COM1 + 1, 0b00000000); // no interrupts
	outb(COM1 + 3, 0b10000000); // set dlab to 1
	outb(COM1 + 0, 0b00000011); // set divisor to 3 (low = 3,
	outb(COM1 + 1, 0b00000000); //                   high = 0)
	outb(COM1 + 2, 0b11000111); // clean and turn on fifo
	outb(COM1 + 3, 0b00000011); // set dlab to 0
	outb(COM1 + 4, 0b00001011); // dont understand

	// loopback test
	outb(COM1 + 4, 0b00011110); // loopback mode
	outb(COM1 + 0, 0xDD);
	if (inb(COM1 + 0) != 0xDD) return false;
	outb(COM1 + 4, 0b00001111); // normal mode

	initialized = true;

	return true;
}

bool xddos_serial_received() {
	return inb(COM1 + 5) & 0b00000001;
}

char xddos_serial_read() {
	if (!initialized) return 0;
	while (xddos_serial_received() == 0);

	return inb(COM1 + 0);
}

bool xddos_serial_is_transmit_empty() {
	return inb(COM1 + 5) & 0b00100000;
}

bool xddos_serial_write(char a) {
	if (!initialized) return 0;

	uint16_t timeout = 1000;
	while (xddos_serial_is_transmit_empty() == 0) {
		if (timeout <= 0) {
			initialized = 0;
			return 0;
		}

		xddos_pit_sleep_ms(1);
		timeout--;
	}

	outb(COM1, a);
	return true;
}

bool xddos_serial_write_text(const char *a) {
	while (*a) {
		if (!xddos_serial_write(*a++)) return false;
	}
	return true;
}
