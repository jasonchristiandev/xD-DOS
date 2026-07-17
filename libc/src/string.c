// some implementations may be taken from musl source code

#include "string.h"
#include <stdint.h>

size_t strlen(const char *str) {
	const char *s;

	for (s = str; *s; ++s);
	return (s - str);
}

void *memcpy(void *restrict dest, const void *restrict src, size_t n) {
	unsigned char *d = dest;
	const unsigned char *s = src;

	typedef uint32_t __attribute__((__may_alias__)) u32;
	uint32_t w, x;

	for (; (uintptr_t) s % 4 && n; n--) *d++ = *s++;

	if ((uintptr_t) d % 4 == 0) {
		for (; n >= 16; s += 16, d += 16, n -= 16) {
			*(u32 *) (d + 0) = *(u32 *) (s + 0);
			*(u32 *) (d + 4) = *(u32 *) (s + 4);
			*(u32 *) (d + 8) = *(u32 *) (s + 8);
			*(u32 *) (d + 12) = *(u32 *) (s + 12);
		}
		if (n & 8) {
			*(u32 *) (d + 0) = *(u32 *) (s + 0);
			*(u32 *) (d + 4) = *(u32 *) (s + 4);
			d += 8;
			s += 8;
		}
		if (n & 4) {
			*(u32 *) (d + 0) = *(u32 *) (s + 0);
			d += 4;
			s += 4;
		}
		if (n & 2) {
			*d++ = *s++;
			*d++ = *s++;
		}
		if (n & 1) {
			*d = *s;
		}
		return dest;
	}

	if (n >= 32) {
		switch ((uintptr_t) d % 4) {
			case 1: {
				w = *(u32 *) s;
				*d++ = *s++;
				*d++ = *s++;
				*d++ = *s++;
				n -= 3;
				for (; n >= 17; s += 16, d += 16, n -= 16) {
					x = *(u32 *) (s + 1);
					*(u32 *) (d + 0) = (w >> 24) | (x << 8);
					w = *(u32 *) (s + 5);
					*(u32 *) (d + 4) = (x >> 24) | (w << 8);
					x = *(u32 *) (s + 9);
					*(u32 *) (d + 8) = (w >> 24) | (x << 8);
					w = *(u32 *) (s + 13);
					*(u32 *) (d + 12) = (x >> 24) | (w << 8);
				}
				break;
			}
			case 2: {
				w = *(u32 *) s;
				*d++ = *s++;
				*d++ = *s++;
				n -= 2;
				for (; n >= 18; s += 16, d += 16, n -= 16) {
					x = *(u32 *) (s + 2);
					*(u32 *) (d + 0) = (w >> 16) | (x << 16);
					w = *(u32 *) (s + 6);
					*(u32 *) (d + 4) = (x >> 16) | (w << 16);
					x = *(u32 *) (s + 10);
					*(u32 *) (d + 8) = (w >> 16) | (x << 16);
					w = *(u32 *) (s + 14);
					*(u32 *) (d + 12) = (x >> 16) | (w << 16);
				}
				break;
			}
			case 3: {
				w = *(u32 *) s;
				*d++ = *s++;
				n -= 1;
				for (; n >= 19; s += 16, d += 16, n -= 16) {
					x = *(u32 *) (s + 3);
					*(u32 *) (d + 0) = (w >> 8) | (x << 24);
					w = *(u32 *) (s + 7);
					*(u32 *) (d + 4) = (x >> 8) | (w << 24);
					x = *(u32 *) (s + 11);
					*(u32 *) (d + 8) = (w >> 8) | (x << 24);
					w = *(u32 *) (s + 15);
					*(u32 *) (d + 12) = (x >> 8) | (w << 24);
				}
				break;
			}
		}
	}
	if (n & 16) {
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
	}
	if (n & 8) {
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
	}
	if (n & 4) {
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
	}
	if (n & 2) {
		*d++ = *s++;
		*d++ = *s++;
	}
	if (n & 1) {
		*d = *s;
	}
	return dest;
}

void *memset(void *dest, int c, size_t n) {
	unsigned char *s = dest;
	size_t k;

	if (!n) return dest;
	s[0] = c;
	s[n - 1] = c;
	if (n <= 2) return dest;
	s[1] = c;
	s[2] = c;
	s[n - 2] = c;
	s[n - 3] = c;
	if (n <= 6) return dest;
	s[3] = c;
	s[n - 4] = c;
	if (n <= 8) return dest;

	k = -(uintptr_t) s & 3;
	s += k;
	n -= k;
	n &= -4;

	typedef uint32_t __attribute__((__may_alias__)) u32;
	typedef uint64_t __attribute__((__may_alias__)) u64;

	u32 c32 = ((u32) -1) / 255 * (unsigned char) c;

	*(u32 *) (s + 0) = c32;
	*(u32 *) (s + n - 4) = c32;
	if (n <= 8) return dest;
	*(u32 *) (s + 4) = c32;
	*(u32 *) (s + 8) = c32;
	*(u32 *) (s + n - 12) = c32;
	*(u32 *) (s + n - 8) = c32;
	if (n <= 24) return dest;
	*(u32 *) (s + 12) = c32;
	*(u32 *) (s + 16) = c32;
	*(u32 *) (s + 20) = c32;
	*(u32 *) (s + 24) = c32;
	*(u32 *) (s + n - 28) = c32;
	*(u32 *) (s + n - 24) = c32;
	*(u32 *) (s + n - 20) = c32;
	*(u32 *) (s + n - 16) = c32;

	k = 24 + ((uintptr_t) s & 4);
	s += k;
	n -= k;

	u64 c64 = c32 | ((u64) c32 << 32);
	for (; n >= 32; n -= 32, s += 32) {
		*(u64 *) (s + 0) = c64;
		*(u64 *) (s + 8) = c64;
		*(u64 *) (s + 16) = c64;
		*(u64 *) (s + 24) = c64;
	}

	return dest;
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
