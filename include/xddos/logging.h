#ifndef LOGGING_H
#define LOGGING_H

#include "xddos/printf.h" // IWYU pragma: keep

#define PRINT(format, ...) printf(format, ##__VA_ARGS__)

#define LOG_INFO(name, format, ...)       \
	do {                                  \
		PRINT("[INFO]      <%s> ", name); \
		PRINT(format, ##__VA_ARGS__);     \
		PRINT("\r\n");                    \
	} while (0)

#define LOG_WARNING(name, format, ...)    \
	do {                                  \
		PRINT("[WARNING]   <%s> ", name); \
		PRINT(format, ##__VA_ARGS__);     \
		PRINT("\r\n");                    \
	} while (0)

#define LOG_ERROR(name, format, ...)      \
	do {                                  \
		PRINT("[ERROR]     <%s> ", name); \
		PRINT(format, ##__VA_ARGS__);     \
		PRINT("\r\n");                    \
	} while (0)

#ifdef DEBUG
#define LOG_DEBUG(name, format, ...)      \
	do {                                  \
		PRINT("[DEBUG]     <%s> ", name); \
		PRINT(format, ##__VA_ARGS__);     \
		PRINT("\r\n");                    \
	} while (0)
#define DELETE_PREV_LINE() \
	do {                   \
		PRINT(" <--\r\n"); \
	} while (0)
#else
#define LOG_DEBUG(name, format, ...) \
	do {                             \
	} while (0)
#define DELETE_PREV_LINE()     \
	do {                       \
		PRINT("\e[1A\e[2K\r"); \
	} while (0)
#endif

#endif // !LOGGING_H