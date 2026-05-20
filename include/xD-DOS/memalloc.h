#ifndef MEMALLOC_H
#define MEMALLOC_H

#include <stddef.h>
#include <stdint.h>

typedef void* (*vmem_provider_t)(size_t pages);

void malloc_init(vmem_provider_t provider, void *initial_heap_base, size_t initial_size);
void *malloc(size_t size);
void free(void *ptr);

#endif // !MEMALLOC_H