#ifndef VMA_H
#define VMA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define HEAP_BASE 0xFFFFFA0000000000ULL
#define HEAP_SIZE (1024 * 16) // 16kb
#define HEAP_LIMIT (0xFFFFFB0000000000ULL - 0xFFFFFA0000000000ULL)

typedef enum : uint8_t {
	VMA_INIT_OK = 0,
	VMA_INIT_OUT_OF_MEMORY = 1,
} vma_init_result_t;

vma_init_result_t vma_init();
void *vma_malloc(size_t size);
void vma_free(void *ptr);
void *vma_calloc(size_t count, size_t size);

#endif