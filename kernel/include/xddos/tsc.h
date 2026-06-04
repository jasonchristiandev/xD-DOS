#ifndef TSC_H
#define TSC_H

#include <stdint.h>

static inline uint64_t xddos_tsc_read() {
	unsigned int lo, hi;
	__asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
	return ((unsigned long long) hi << 32) | lo;
}

#endif // !TSC_H