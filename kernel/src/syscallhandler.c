#include "xddos/syscallhandler.h"
#include "xddos/serial.h"
#include "xddos/syscallnums.h"
// #include "xddos/vma.h"

#define MSR_EFER 0xC0000080
#define EFER_SCE (1 << 0)
#define MSR_STAR 0xC0000081
#define MSR_LSTAR 0xC0000082
#define MSR_FMASK 0xC0000084
#define KERNEL_CS 0x08
#define USER_CS 0x1B

extern uint64_t global_kernel_stack;
uint8_t syscall_stack[4096];

void syscall_init() {
	uint32_t low, high;
	__asm__ volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(MSR_EFER));
	low |= EFER_SCE;
	__asm__ volatile("wrmsr" : : "a"(low), "d"(high), "c"(MSR_EFER));

	extern void syscall_entry(void);
	uint64_t lstar = (uint64_t) syscall_entry;
	__asm__ volatile("wrmsr" : : "a"((uint32_t) lstar), "d"((uint32_t) (lstar >> 32)), "c"(MSR_LSTAR));

	global_kernel_stack = (uint64_t) &syscall_stack[4096];

	low = 0;
	high = (KERNEL_CS << 0) | (USER_CS << 16);
	__asm__ volatile("wrmsr" : : "a"(low), "d"(high), "c"(MSR_STAR));

	uint32_t fmask = 0x200;
	__asm__ volatile("wrmsr" : : "a"(fmask), "d"(0), "c"(MSR_FMASK));
}

uint64_t syscall_handler(uint64_t id, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
	(void) arg3;
	switch (id) {
		case SYSCALL_WRITE:
			if (arg1 == 0) {
				return serial_write((char) arg2);
			}
			return -1;
		case SYSCALL_SBRK:
			// return (uint64_t) vma_alloc_pages(arg1);
			return -1;
		default:
			return -1;
	}
}