#ifndef LOGGING_H
#define LOGGING_H

void LOG_INFO(const char *name, const char *format, ...);
void LOG_WARNING(const char *name, const char *format, ...);
void LOG_ERROR(const char *name, const char *format, ...);
void LOG_DEBUG(const char *name, const char *format, ...);

#endif // !LOGGING_H