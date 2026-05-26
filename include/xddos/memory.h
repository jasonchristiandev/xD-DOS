#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

void *memory_copy(void *restrict dest, const void *restrict src, size_t n);
void *memory_set(void *s, int c, size_t n);
void *memory_move(void *dest, const void *src, size_t n);
int memory_cmp(const void *s1, const void *s2, size_t n);

#endif // !MEMORY_H