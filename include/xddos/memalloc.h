#ifndef MEMALLOC_H
#define MEMALLOC_H

#include <stddef.h>

typedef void* (*vmem_provider)(size_t pages);

void xddos_memalloc_init(vmem_provider provider, void *initial_heap_base, size_t initial_size);
void *xddos_memalloc_malloc(size_t size);
void xddos_memalloc_free(void *ptr);
void *xddos_memalloc_calloc(size_t n, size_t size);

#endif // !MEMALLOC_H