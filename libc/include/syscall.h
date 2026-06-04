#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

uint64_t syscall(uint64_t vector_id, uint64_t arg1, uint64_t arg2, uint64_t arg3);

#endif // !SYSCALL_H