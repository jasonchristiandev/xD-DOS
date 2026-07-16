#include "xddos/asm.h"

void outl(uint16_t port, uint32_t val) {
	__asm__ volatile("outl %0, %1" : : "a"(val), "Nd"(port));
}

uint32_t inl(uint16_t port) {
	uint32_t ret;
	__asm__ volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

void outb(uint16_t port, uint8_t val) {
	__asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

uint8_t inb(uint16_t port) {
	uint8_t ret;
	__asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

void outw(uint16_t port, uint16_t val) {
	__asm__ volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}

uint16_t inw(uint16_t port) {
	uint16_t ret;
	__asm__ volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

void insw(uint16_t port, void *addr, uint32_t count) {
	__asm__ volatile("cld; rep insw" : "+D"(addr), "+c"(count) : "d"(port) : "memory");
}

void io_wait() {
	__asm__ volatile("outb %%al, $0x80" : : "a"(0));
}

void invlpg(uint64_t addr) {
	__asm__ volatile("invlpg (%0)" ::"r"(addr) : "memory");
}

uint64_t rdmsr(uint32_t msr) {
	uint32_t lo;
	uint32_t hi;

	__asm__ __volatile__(
		"rdmsr"
		: "=a"(lo), "=d"(hi)
		: "c"(msr));

	return ((uint64_t) hi << 32) | lo;
}

void wrmsr(uint32_t msr, uint64_t value) {
	uint32_t lo = (uint32_t) value;
	uint32_t hi = (uint32_t) (value >> 32);

	__asm__ __volatile__(
		"wrmsr"
		:
		: "c"(msr), "a"(lo), "d"(hi)
		: "memory");
}