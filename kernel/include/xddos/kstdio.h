#ifndef KSTDIO_H
#define KSTDIO_H

#include <stdarg.h>
#include <stddef.h>

int xddos_kstdio_putchar(char ch);
int xddos_kstdio_vprintf(const char *format, va_list args);
int xddos_kstdio_vsprintf(char *str, const char *format, va_list args);
int xddos_kstdio_vsnprintf(char *str, size_t size, const char *format, va_list args);
void xddos_kstdio_printf(const char *format, ...);
int xddos_kstdio_sprintf(char *str, const char *format, ...);
int xddos_kstdio_snprintf(char *str, size_t size, const char *format, ...);

#endif // !KSTDIO_H