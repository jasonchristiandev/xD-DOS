#ifndef VMA_H
#define VMA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void xddos_vma_init(uint64_t heap_base);
void *xddos_vma_malloc(size_t size);
void xddos_vma_free(void *ptr);
void *xddos_vma_calloc(size_t count, size_t size);

#endif