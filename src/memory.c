#include "xddos/memory.h"
#include <stdint.h>

void *memory_copy(void *restrict dest, const void *restrict src, size_t n) {
	uint8_t *restrict pdest = dest;
	const uint8_t *restrict psrc = src;

	for (size_t i = 0; i < n; i++) {
		pdest[i] = psrc[i];
	}

	return dest;
}

void *memory_set(void *s, int c, size_t n) {
	uint8_t *p = s;

	for (size_t i = 0; i < n; i++) {
		p[i] = (uint8_t) c;
	}

	return s;
}

void *memory_move(void *dest, const void *src, size_t n) {
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

int memory_cmp(const void *a, const void *b, size_t n) {
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