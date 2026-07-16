#include "string.h"
#include <stdint.h>

size_t strlen(const char *str) {
	const char *s;

	for (s = str; *s; ++s);
	return (s - str);
}

void *memcpy(void *restrict dest, const void *restrict src, size_t n) {
	uint8_t *restrict pdest = dest;
	const uint8_t *restrict psrc = src;

	for (size_t i = 0; i < n; i++) {
		pdest[i] = psrc[i];
	}

	return dest;
}

void *memset(void *s, int c, size_t n) {
	uint8_t *p = s;

	for (size_t i = 0; i < n; i++) {
		p[i] = (uint8_t) c;
	}

	return s;
}

void *memmove(void *dest, const void *src, size_t n) {
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

int memcmp(const void *a, const void *b, size_t n) {
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

int strncmp(const char *a, const char *b, register size_t n) {
	register unsigned char u1, u2;

	while (n-- > 0) {
		u1 = (unsigned char) *a++;
		u2 = (unsigned char) *b++;
		if (u1 != u2) return u1 - u2;
		if (u1 == '\0') return 0;
	}

	return 0;
}
