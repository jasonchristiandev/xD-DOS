#ifndef MEMALLOC_H
#define MEMALLOC_H

#include <stddef.h>
#include <stdint.h>

typedef void* (*vmem_provider)(size_t pages);

void memalloc_init(vmem_provider provider, void *initial_heap_base, size_t initial_size);
void *memalloc_malloc(size_t size);
void memalloc_free(void *ptr);
void *memalloc_calloc(size_t n, size_t size);

#endif // !MEMALLOC_H