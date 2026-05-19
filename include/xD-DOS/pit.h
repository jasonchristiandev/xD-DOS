#ifndef PIT_H
#define PIT_H

#include "xD-DOS/asm.h"
#include <stdint.h>

#define PIT_CHANNEL_2 0x42
#define PIT_COMMAND 0x43
#define PIT_PORT_B 0x61

static void sleep_ms(uint32_t ms) {
	for (uint32_t i = 0; i < ms; i++) {
		outb(PIT_COMMAND, 0xB2);
		
		// Set count to 1193 for approx 1ms
		outb(PIT_CHANNEL_2, 0xA9);
		outb(PIT_CHANNEL_2, 0x04);
		
		// Start countdown
		uint8_t port_b = inb(PIT_PORT_B);
		outb(PIT_PORT_B, port_b | 1);
		
		while ((inb(PIT_PORT_B) & 0x20) == 0);
		
		// Stop
		outb(PIT_PORT_B, inb(PIT_PORT_B) & ~1);
	}
}

static void sleep_s(uint32_t s) {
	sleep_ms(s * 1000);
}

#endif // !PIT_H