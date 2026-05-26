#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h> // IWYU pragma: keep

#define LOG_INFO(name, format, ...)       \
	do {                                  \
		printf("[INFO]      <%s> ", name); \
		printf(format, ##__VA_ARGS__);     \
		printf("\r\n");                    \
	} while (0)

#define LOG_WARNING(name, format, ...)    \
	do {                                  \
		printf("[WARNING]   <%s> ", name); \
		printf(format, ##__VA_ARGS__);     \
		printf("\r\n");                    \
	} while (0)

#define LOG_ERROR(name, format, ...)      \
	do {                                  \
		printf("[ERROR]     <%s> ", name); \
		printf(format, ##__VA_ARGS__);     \
		printf("\r\n");                    \
	} while (0)

#ifdef DEBUG
#define LOG_DEBUG(name, format, ...)      \
	do {                                  \
		printf("[DEBUG]     <%s> ", name); \
		printf(format, ##__VA_ARGS__);     \
		printf("\r\n");                    \
	} while (0)
#define DELETE_PREV_LINE() \
	do {                   \
		printf(" <--\r\n"); \
	} while (0)
#else
#define LOG_DEBUG(name, format, ...) \
	do {                             \
	} while (0)
#define DELETE_PREV_LINE()     \
	do {                       \
		printf("\e[1A\e[2K\r"); \
	} while (0)
#endif

#endif // !LOGGING_H