#ifndef PS2_H
#define PS2_H

#include <stdint.h>

void ps2_mouse_write(uint8_t value);
uint8_t ps2_mouse_read();
void ps2_mouse_init(uint8_t lapic);

#endif // !PS2_H