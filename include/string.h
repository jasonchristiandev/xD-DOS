#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdint.h>

static inline size_t strlen(const char *str) {
	const char *s;
	
	for (s = str; *s; ++s);
	return (s - str);
}

static inline void *memcopy(void *restrict dest, const void *restrict src, size_t n) {
	uint8_t *restrict pdest = dest;
	const uint8_t *restrict psrc = src;

	for (size_t i = 0; i < n; i++) {
		pdest[i] = psrc[i];
	}

	return dest;
}

static inline void *memset(void *s, int c, size_t n) {
	uint8_t *p = s;

	for (size_t i = 0; i < n; i++) {
		p[i] = (uint8_t) c;
	}

	return s;
}

static inline void *memmove(void *dest, const void *src, size_t n) {
	uint8_t *pdest = dest;
	const uint8_t *psrc = src;

	if ((uintptr_t) src > (uintptr_t) dest) {
		for (size_t i = 0; i < n; i++) {
			pdest[i] = psrc[i];
		}
	} else if ((uintptr_t) src < (uintptr_t) dest) {
		for (size_t i = n; i > 0; i--) {
			pdest[i - 1] = psrc[i - 1];
		}
	}

	return dest;
}

static inline int memcmp(const void *a, const void *b, size_t n) {
	const uint8_t *pa = a;
	const uint8_t *pb = b;

	while (n--) {
		if (*pa != *pb) {
			return (int) *pa - (int) *pb;
		}
		pa++;
		pb++;
	}

	return 0;
}

#endif // !STRING_H