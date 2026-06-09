#include "xddos/tsc.h"
#include <stdint.h>

uint64_t xddos_tsc_read() {
	unsigned int lo, hi;
	__asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
	return ((unsigned long long) hi << 32) | lo;
}