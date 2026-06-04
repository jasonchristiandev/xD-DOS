#include <syscall.h>

uint64_t syscall(uint64_t vector_id, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
	uint64_t ret;
	__asm__ __volatile__(
		"mov %1, %%rax\n\t"
		"mov %2, %%rdi\n\t"
		"mov %3, %%rsi\n\t"
		"mov %4, %%rdx\n\t"
		"syscall\n\t"
		"mov %%rax, %0"
		: "=r"(ret)
		: "r"(vector_id), "r"(arg1), "r"(arg2), "r"(arg3)
		: "rax", "rdi", "rsi", "rdx", "rcx", "r11", "memory");
	return ret;
}