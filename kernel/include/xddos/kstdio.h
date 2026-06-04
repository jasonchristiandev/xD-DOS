#ifndef KSTDIO_H
#define KSTDIO_H

#include <stdarg.h>
#include <stddef.h>

int putchar(char ch);
int vprintf(const char *format, va_list args);
int vsprintf(char *str, const char *format, va_list args);
int vsnprintf(char *str, size_t size, const char *format, va_list args);
void printf(const char *format, ...);
int sprintf(char *str, const char *format, ...);
int snprintf(char *str, size_t size, const char *format, ...);

#endif // !KSTDIO_H