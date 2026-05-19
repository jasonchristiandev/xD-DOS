#ifndef PRINTF_H
#define PRINTF_H

#include "xD-DOS/serial.h"
#include <stdarg.h>
#include <stdbool.h>

static inline void printf_itoa(unsigned long long n, char *str, int base, bool signed_val) {
	char *p = str;
	char *pa, *pb;
	unsigned long long decimal = n;
	
	if (signed_val && base == 10 && (long long) n < 0) {
		*p++ = L'-';
		decimal = -(long long) n;
	}
	
	char *first_digit = p;
	do {
		int digit = decimal % base;
		*p++ = (digit < 10) ? (L'0' + digit) : (L'A' + digit - 10);
	} while (decimal /= base);
	*p = L'\0';
	
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

static inline void printf(const char *format, ...) {
	va_list args;
	va_start(args, format);
	
	char buf[128];
	for (int i = 0; format[i] != L'\0'; i++) {
		if (format[i] == L'%' && format[i + 1] != L'\0') {
			i++;
			
			bool is64 = false;
			if (format[i] == L'l' && format[i + 1] == L'l') {
				is64 = true;
				i += 2;
			}
			
			switch (format[i]) {
				case L's': {
					char *s = va_arg(args, char *);
					serial_write_text(s);
					break;
				}
				case L'd': {
					if (is64) {
						printf_itoa(va_arg(args, long long), buf, 10, true);
					} else {
						printf_itoa(va_arg(args, int), buf, 10, true);
					}
					
					serial_write_text(buf);
					break;
				}
				case L'u': {
					if (is64) {
						printf_itoa(va_arg(args, unsigned long long), buf, 10, false);
					} else {
						printf_itoa(va_arg(args, unsigned int), buf, 10, false);
					}
					
					serial_write_text(buf);
					break;
				}
				case L'x': {
					if (is64) {
						printf_itoa(va_arg(args, unsigned long long), buf, 16, false);
					} else {
						printf_itoa(va_arg(args, unsigned int), buf, 16, false);
					}
					
					serial_write_text(buf);
					break;
				}
			}
		} else {
			char single[2] = {format[i], L'\0'};
			serial_write_text(single);
		}
	}
	va_end(args);
}

#endif // !PRINTF_H