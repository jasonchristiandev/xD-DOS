#include "xddos/syscallhandler.h"
#include "xddos/serial.h"
#include "xddos/syscallnums.h"
#include "xddos/vma.h"

uint64_t xddos_syscall_handler(uint64_t id, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
	(void) arg3;
	switch (id) {
		case SYSCALL_WRITE:
			if (arg1 == 0) {
				return xddos_serial_write((char) arg2);
			}
			return -1;
		case SYSCALL_SBRK:
			return (uint64_t) xddos_vma_alloc_pages(arg1);
		default:
			return -1;
	}
}