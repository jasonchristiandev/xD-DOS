#ifndef PIT_H
#define PIT_H

#include <stdint.h>

#define PIT_CHANNEL_2 0x42
#define PIT_COMMAND 0x43
#define PIT_PORT_B 0x61

void xddos_pit_sleep_ms(uint32_t ms);

#endif // !PIT_H