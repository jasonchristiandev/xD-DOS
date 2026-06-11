#include "xddos/logging.h"
#include "xddos/kstdio.h"
#define NAME_MAX_LENGTH 10

static char *truncate(const char *name) {
	static char res[NAME_MAX_LENGTH + 3];

	res[0] = '<';

	int i;
	for (i = 0; name[i] != '\0'; i++) {
		if (i == NAME_MAX_LENGTH) {
			for (int j = NAME_MAX_LENGTH; j > NAME_MAX_LENGTH - 3; j--) res[j] = '.';
			break;
		}
		res[i + 1] = name[i];
	}
	res[++i] = '>';
	for (i++; i < NAME_MAX_LENGTH + 3; i++) {
		res[i] = ' ';
	}
	res[NAME_MAX_LENGTH + 3 - 1] = '\0';

	return res;
}

static void internal_log_write(const char *prefix, const char *name, const char *format, va_list args) {
	char log_buf[512];

	xddos_kstdio_vsnprintf(log_buf, sizeof(log_buf), format, args);
	xddos_kstdio_printf("%s %s ", prefix, truncate(name));

	for (size_t i = 0; log_buf[i] != '\0'; i++) {
		if (log_buf[i] == '\n') {
			xddos_kstdio_putchar('\n');
			if (log_buf[i + 1] != '\0') xddos_kstdio_printf("%s %s ", prefix, truncate(name));
		} else {
			xddos_kstdio_putchar(log_buf[i]);
		}
	}

	xddos_kstdio_printf("\r\n");
}

void LOG_INFO(const char *name, const char *format, ...) {
	va_list args;
	va_start(args, format);
	internal_log_write("[INFO]   ", name, format, args);
	va_end(args);
}

void LOG_WARNING(const char *name, const char *format, ...) {
	va_list args;
	va_start(args, format);
	internal_log_write("[WARNING]", name, format, args);
	va_end(args);
}

void LOG_ERROR(const char *name, const char *format, ...) {
	va_list args;
	va_start(args, format);
	internal_log_write("[ERROR]  ", name, format, args);
	va_end(args);
}

#if DEBUG == 1
void LOG_DEBUG(const char *name, const char *format, ...) {
	va_list args;
	va_start(args, format);
	internal_log_write("[DEBUG]  ", name, format, args);
	va_end(args);
}
#else
void LOG_DEBUG(const char *name, const char *format, ...) {
	(void) name;
	(void) format;
}
#endif