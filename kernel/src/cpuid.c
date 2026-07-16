#include <stdbool.h>
#include <stdint.h>

typedef struct {
	uint32_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
} cpuid_result_t;

static inline cpuid_result_t cpuid(uint32_t leaf, uint32_t subleaf) {
	cpuid_result_t r;

	__asm__ __volatile__(
		"cpuid"
		: "=a"(r.eax), "=b"(r.ebx), "=c"(r.ecx), "=d"(r.edx)
		: "a"(leaf), "c"(subleaf));

	return r;
}

bool cpuid_msr() {
	cpuid_result_t r = cpuid(1, 0);
	return (r.edx >> 5) & 1;
}

bool cpuid_apic() {
	cpuid_result_t r = cpuid(1, 0);
	return (r.edx >> 9) & 1;
}
