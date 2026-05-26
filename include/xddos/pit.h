#ifndef PIT_H
#define PIT_H

#include "xddos/asm.h"
#include <stdint.h>

#define PIT_CHANNEL_2 0x42
#define PIT_COMMAND 0x43
#define PIT_PORT_B 0x61

#define SLEEP pit_sleep_ms

void pit_sleep_ms(uint32_t ms);

#endif // !PIT_H