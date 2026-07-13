#include "xddos/pit.h"
#include "xddos/asm.h"

#define PIT_CHANNEL_2 0x42
#define PIT_COMMAND 0x43
#define PIT_PORT_B 0x61

void pit_sleep_ms(uint64_t ms) {
	for (uint64_t i = 0; i < ms; i++) {
		// 10  channel 2
		// 11  low high
		// 001 hardware retriggerable one shot
		// 0   binary mode
		outb(PIT_COMMAND, 0b10110010);

		outb(PIT_CHANNEL_2, 0xA9); // 1000 pit cycle approx 1193 ms
		outb(PIT_CHANNEL_2, 0x04); // 1193 = 0x04A9

		uint8_t port_b = inb(PIT_PORT_B);
		outb(PIT_PORT_B, port_b | 1); // enable gate

		while ((inb(PIT_PORT_B) & 0b10000) == 0); // bit 5 = status

		outb(PIT_PORT_B, inb(PIT_PORT_B) & ~1); // disable gate
	}
}