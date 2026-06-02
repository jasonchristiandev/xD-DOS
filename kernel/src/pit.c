#include "xddos/pit.h"
#include "xddos/asm.h"

void xddos_pit_sleep_ms(uint32_t ms) {
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