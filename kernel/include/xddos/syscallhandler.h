#ifndef SYSCALLHANDLER_H
#define SYSCALLHANDLER_H

#include <stdint.h>

void xddos_syscall_init();
uint64_t xddos_syscall_handler(uint64_t id, uint64_t arg1, uint64_t arg2, uint64_t arg3);

#endif // !SYSCALLHANDLER_H