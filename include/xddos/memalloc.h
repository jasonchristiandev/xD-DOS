#ifndef MEMALLOC_H
#define MEMALLOC_H

#include <stddef.h>
#include <stdint.h>

typedef void* (*vmem_provider)(size_t pages);

void memalloc_init(vmem_provider provider, void *initial_heap_base, size_t initial_size);
void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t n, size_t size);

#endif // !MEMALLOC_H