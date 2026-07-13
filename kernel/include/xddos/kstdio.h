#ifndef KSTDIO_H
#define KSTDIO_H

#include <stdarg.h>
#include <stddef.h>

int kstdio_putchar(char ch);
int kstdio_vprintf(const char *format, va_list args);
int kstdio_vsprintf(char *str, const char *format, va_list args);
int kstdio_vsnprintf(char *str, size_t size, const char *format, va_list args);
void kstdio_printf(const char *format, ...);
int kstdio_sprintf(char *str, const char *format, ...);
int kstdio_snprintf(char *str, size_t size, const char *format, ...);

#endif // !KSTDIO_H