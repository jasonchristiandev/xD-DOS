#ifndef VMA_H
#define VMA_H

#include <stddef.h>
#include <stdint.h>

void vma_init(uint64_t heap_base);
void *vma_alloc_pages(size_t pages);

#endif