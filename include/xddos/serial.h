#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

uint8_t xddos_serial_init();
uint8_t xddos_serial_received();
char xddos_serial_read();
uint8_t xddos_serial_is_transmit_empty();
uint8_t xddos_serial_write(char a);
uint8_t xddos_serial_write_text(char *a);

#endif // !SERIAL_H