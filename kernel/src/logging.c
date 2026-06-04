#include "xddos/logging.h"
#include <stdio.h>
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

void LOG_INFO(const char *name, const char *format, ...) {
	va_list args;
	va_start(args, format);
	printf("[INFO]      %s ", truncate(name));
	vprintf(format, args);
	printf("\r\n");
	va_end(args);
}

void LOG_WARNING(const char *name, const char *format, ...) {
	va_list args;
	va_start(args, format);
	printf("[WARNING]   %s ", truncate(name));
	vprintf(format, args);
	printf("\r\n");
	va_end(args);
}

void LOG_ERROR(const char *name, const char *format, ...) {
	va_list args;
	va_start(args, format);
	printf("[ERROR]     %s ", truncate(name));
	vprintf(format, args);
	printf("\r\n");
	va_end(args);
}

void LOG_DEBUG(const char *name, const char *format, ...) {
	va_list args;
	va_start(args, format);
	printf("[DEBUG]     %s ", truncate(name));
	vprintf(format, args);
	printf("\r\n");
	va_end(args);
}
