#ifndef PRINT_C
#define PRINT_C

#include "xddos/serial.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

static void printf_itoa(uint64_t n, char *str, uint8_t base, uint8_t signed_val) {
	char *p = str;
	char *pa, *pb;
	uint64_t decimal = n;

	if (signed_val && base == 10 && (int64_t) n < 0) {
		*p++ = '-';
		decimal = -(int64_t) n;
	}

	char *first_digit = p;
	do {
		uint8_t digit = decimal % base;
		*p++ = (digit < 10) ? ('0' + digit) : ('A' + digit - 10);
	} while (decimal /= base);
	*p = '\0';

	pa = first_digit;
	pb = p - 1;
	while (pa < pb) {
		char tmp = *pa;
		*pa = *pb;
		*pb = tmp;
		pa++;
		pb--;
	}
}

void printf(const char *format, ...) {
	va_list args;
	va_start(args, format);

	char buf[128];
	for (size_t i = 0; format[i] != '\0'; i++) {
		if (format[i] == '%' && format[i + 1] != '\0') {
			i++;

			uint8_t is64 = 0;
			if (format[i] == 'l' && format[i + 1] == 'l') {
				is64 = 1;
				i += 2;
			} else if (format[i] == 'l' && format[i + 1] != 'l') {
				is64 = 1;
				i++;
			}

			switch (format[i]) {
				case 's': {
					char *s = va_arg(args, char *);
					serial_write_text(s);
					break;
				}
				case 'd': {
					if (is64) {
						printf_itoa(va_arg(args, int64_t), buf, 10, 1);
					} else {
						printf_itoa(va_arg(args, int), buf, 10, 1);
					}

					serial_write_text(buf);
					break;
				}
				case 'u': {
					if (is64) {
						printf_itoa(va_arg(args, uint64_t), buf, 10, 0);
					} else {
						printf_itoa(va_arg(args, uint32_t), buf, 10, 0);
					}

					serial_write_text(buf);
					break;
				}
				case 'x': {
					if (is64) {
						printf_itoa(va_arg(args, uint64_t), buf, 16, 0);
					} else {
						printf_itoa(va_arg(args, uint32_t), buf, 16, 0);
					}

					serial_write_text(buf);
					break;
				}
			}
		} else {
			char single[2] = {format[i], '\0'};
			serial_write_text(single);
		}
	}
	va_end(args);
}

#endif // !PRINT_C