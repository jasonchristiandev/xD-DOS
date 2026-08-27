#ifndef __XDDOS_LOGGING_H
#define __XDDOS_LOGGING_H

void LOG_INFO(const char *name, const char *format, ...);
void LOG_WARNING(const char *name, const char *format, ...);
void LOG_ERROR(const char *name, const char *format, ...);
void LOG_DEBUG(const char *name, const char *format, ...);

#endif // !__XDDOS_LOGGING_H