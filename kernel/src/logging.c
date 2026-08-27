#include "xddos/logging.h"
#include "xddos/define.h"
#include "xddos/kstdio.h"

#define NAME_MAX_LENGTH 12
#define LOG_BUF_SIZE 256

#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_GREEN "\033[32m"
#define COLOR_BLUE "\033[34m"

void format_name(const char *name, char *out, size_t size) {
	if (size < 4) return;

	out[0] = '<';
	size_t i = 1;

	size_t len = 0;
	while (name[len] != '\0') len++;

	if (len > NAME_MAX_LENGTH) {
		for (size_t j = 0; j < NAME_MAX_LENGTH - 3; j++) {
			out[i++] = name[j];
		}
		out[i++] = '.';
		out[i++] = '.';
		out[i++] = '.';
	} else {
		for (size_t j = 0; j < len; j++) {
			out[i++] = name[j];
		}
	}

	out[i++] = '>';

	while (i < NAME_MAX_LENGTH + 2 && i < size - 1) {
		out[i++] = ' ';
	}

	out[i] = '\0';
}

void write(const char *prefix, const char *name, const char *format, va_list args) {
	char buf[LOG_BUF_SIZE];
	char formatted[NAME_MAX_LENGTH + 3];

	format_name(name, formatted, sizeof(formatted));
	kstdio_vsnprintf(buf, sizeof(buf), format, args);

	kstdio_printf("%s %s ", prefix, formatted);

	size_t indent = 9 + 1 + (NAME_MAX_LENGTH + 2) + 1;

	for (size_t i = 0; buf[i] != '\0'; i++) {
		if (buf[i] == '\r') continue;

		if (buf[i] == '\n') {
			if (buf[i + 1] != '\0') {
				kstdio_printf(COLOR_RESET "\r\n");
				for (size_t s = 0; s < indent; s++) {
					kstdio_putchar(' ');
				}
			}
		} else {
			kstdio_putchar(buf[i]);
		}
	}

	kstdio_printf(COLOR_RESET "\r\n");
}

void LOG_INFO(const char *name, const char *format, ...) {
	va_list args;
	va_start(args, format);
	write(COLOR_GREEN "[INFO]   " COLOR_RESET, name, format, args);
	va_end(args);
}

void LOG_WARNING(const char *name, const char *format, ...) {
	va_list args;
	va_start(args, format);
	write(COLOR_YELLOW "[WARNING]" COLOR_RESET, name, format, args);
	va_end(args);
}

void LOG_ERROR(const char *name, const char *format, ...) {
	va_list args;
	va_start(args, format);
	write(COLOR_RED "[ERROR]  " COLOR_RESET, name, format, args);
	va_end(args);
}

NO_OPTIMIZE void LOG_DEBUG(const char *name, const char *format, ...) {
	va_list args;
	va_start(args, format);
	if (VERBOSE) write(COLOR_BLUE "[DEBUG]  " COLOR_RESET, name, format, args);
	va_end(args);
}