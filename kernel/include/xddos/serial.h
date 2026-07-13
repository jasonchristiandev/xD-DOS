#ifndef SERIAL_H
#define SERIAL_H

#include <stdbool.h>

bool serial_init();
bool serial_received();
char serial_read();
bool serial_is_transmit_empty();
bool serial_write(char a);
bool serial_write_text(const char *a);

#endif // !SERIAL_H