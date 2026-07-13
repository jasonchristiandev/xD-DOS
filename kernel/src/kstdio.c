#include "xddos/kstdio.h"
#include "xddos/serial.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

int kstdio_putchar(char ch) {
	return serial_write(ch) ? ch : '\0';
}

static void itoa(uint64_t n, char *str, uint8_t base, uint8_t signed_val) {
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

int kstdio_vprintf(const char *format, va_list args) {
	char buf[128];
	int result = kstdio_vsnprintf(buf, sizeof(buf), format, args);
	serial_write_text(buf);
	return result;
}

int kstdio_vsprintf(char *str, const char *format, va_list args) {
	return kstdio_vsnprintf(str, (size_t) -1, format, args);
}

int kstdio_vsnprintf(char *str, size_t size, const char *format, va_list args) {
	if (str == NULL || size == 0) {
		return 0;
	}

	size_t idx = 0;
	char buf[128];

	for (size_t i = 0; format[i] != '\0'; i++) {
		if (format[i] == '%' && format[i + 1] != '\0') {
			i++;

			bool is64 = false;
			if (format[i] == 'l' && format[i + 1] == 'l') {
				is64 = true;
				i += 2;
			} else if (format[i] == 'l' && format[i + 1] != 'l') {
				is64 = true;
				i++;
			}

			char *src = NULL;

			switch (format[i]) {
				case 's': {
					src = va_arg(args, char *);
					break;
				}
				case 'd': {
					if (is64) {
						itoa(va_arg(args, int64_t), buf, 10, 1);
					} else {
						itoa(va_arg(args, int), buf, 10, 1);
					}
					src = buf;
					break;
				}
				case 'u': {
					if (is64) {
						itoa(va_arg(args, uint64_t), buf, 10, 0);
					} else {
						itoa(va_arg(args, uint32_t), buf, 10, 0);
					}
					src = buf;
					break;
				}
				case 'x': {
					if (is64) {
						itoa(va_arg(args, uint64_t), buf, 16, 0);
					} else {
						itoa(va_arg(args, uint32_t), buf, 16, 0);
					}
					src = buf;
					break;
				}
			}

			if (src != NULL) {
				for (size_t j = 0; src[j] != '\0'; j++) {
					if (idx < size - 1) {
						str[idx++] = src[j];
					} else {
						break;
					}
				}
			}
		} else {
			if (idx < size - 1) {
				str[idx++] = format[i];
			}
		}
	}

	str[idx] = '\0';
	return idx;
}

void kstdio_printf(const char *format, ...) {
	va_list args;
	va_start(args, format);
	kstdio_vprintf(format, args);
	va_end(args);
}

int kstdio_sprintf(char *str, const char *format, ...) {
	va_list args;
	va_start(args, format);
	int result = kstdio_vsnprintf(str, (size_t) -1, format, args);
	va_end(args);
	return result;
}

int kstdio_snprintf(char *str, size_t size, const char *format, ...) {
	va_list args;
	va_start(args, format);
	int result = kstdio_vsnprintf(str, size, format, args);
	va_end(args);
	return result;
}