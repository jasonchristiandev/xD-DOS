// some implementations may be taken from musl source code

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

void *memset(void *dest, int c, size_t n) {
	unsigned char *s = dest;
	size_t k;

	/* Fill head and tail with minimal branching. Each
	 * conditional ensures that all the subsequently used
	 * offsets are well-defined and in the dest region. */

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

	/* Advance pointer to align it at a 4-byte boundary,
	 * and truncate n to a multiple of 4. The previous code
	 * already took care of any head/tail that get cut off
	 * by the alignment. */

	k = -(uintptr_t) s & 3;
	s += k;
	n -= k;
	n &= -4;

#ifdef __GNUC__
	typedef uint32_t __attribute__((__may_alias__)) u32;
	typedef uint64_t __attribute__((__may_alias__)) u64;

	u32 c32 = ((u32) -1) / 255 * (unsigned char) c;

	/* In preparation to copy 32 bytes at a time, aligned on
	 * an 8-byte bounary, fill head/tail up to 28 bytes each.
	 * As in the initial byte-based head/tail fill, each
	 * conditional below ensures that the subsequent offsets
	 * are valid (e.g. !(n<=24) implies n>=28). */

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

	/* Align to a multiple of 8 so we can fill 64 bits at a time,
	 * and avoid writing the same bytes twice as much as is
	 * practical without introducing additional branching. */

	k = 24 + ((uintptr_t) s & 4);
	s += k;
	n -= k;

	/* If this loop is reached, 28 tail bytes have already been
	 * filled, so any remainder when n drops below 32 can be
	 * safely ignored. */

	u64 c64 = c32 | ((u64) c32 << 32);
	for (; n >= 32; n -= 32, s += 32) {
		*(u64 *) (s + 0) = c64;
		*(u64 *) (s + 8) = c64;
		*(u64 *) (s + 16) = c64;
		*(u64 *) (s + 24) = c64;
	}
#else
	/* Pure C fallback with no aliasing violations. */
	for (; n; n--, s++) *s = c;
#endif

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
