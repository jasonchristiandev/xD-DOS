#ifndef SERIAL_H
#define SERIAL_H

int serial_init();
int serial_received();
char serial_read();
int serial_is_transmit_empty();
void serial_write(char a);
void serial_write_text(char *a);

#endif // !SERIAL_H