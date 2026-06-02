#ifndef SERIAL_H
#define SERIAL_H

#include <stdbool.h>

bool xddos_serial_init();
bool xddos_serial_received();
char xddos_serial_read();
bool xddos_serial_is_transmit_empty();
bool xddos_serial_write(char a);
bool xddos_serial_write_text(char *a);

#endif // !SERIAL_H