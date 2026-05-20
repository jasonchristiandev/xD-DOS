#include "xD-DOS/memalloc.h"
#include <string.h>

#define ALIGNMENT 16
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define PAGE_SIZE 4096

typedef struct block_header {
	size_t size;
	struct block_header *next;
	struct block_header *prev;
} block_header_t;

#define BLOCK_FREE_BIT ((size_t) 1)
#define GET_SIZE(block) ((block)->size & ~BLOCK_FREE_BIT)
#define IS_FREE(block) ((block)->size & BLOCK_FREE_BIT)

#define SET_ALLOCATED(block) ((block)->size &= ~BLOCK_FREE_BIT)
#define SET_FREE(block) ((block)->size |= BLOCK_FREE_BIT)

typedef struct {
	size_t size;
} block_footer_t;

static block_header_t *free_list_head = NULL;
static vmem_provider_t raw_vmem_provider = NULL;
static uintptr_t heap_end_addr = 0;

static inline block_footer_t *get_footer(block_header_t *block) {
	return (block_footer_t *) ((uintptr_t) block + sizeof(block_header_t) + GET_SIZE(block));
}

static inline block_header_t *get_prev_phys(block_header_t *block, uintptr_t heap_base) {
	if ((uintptr_t) block == heap_base) return NULL;
	block_footer_t *prev_footer = (block_footer_t *) ((uintptr_t) block - sizeof(block_footer_t));
	return (block_header_t *) ((uintptr_t) block - sizeof(block_header_t) - prev_footer->size - sizeof(block_footer_t));
}

static inline block_header_t *get_next_phys(block_header_t *block) {
	uintptr_t next_addr = (uintptr_t) get_footer(block) + sizeof(block_footer_t);
	if (next_addr >= heap_end_addr) return NULL;
	return (block_header_t *) next_addr;
}

static void atf(block_header_t *block) {
	SET_FREE(block);
	block->next = free_list_head;
	block->prev = NULL;
	if (free_list_head) {
		free_list_head->prev = block;
	}
	free_list_head = block;
}

static void rff(block_header_t *block) {
	if (block == free_list_head) {
		free_list_head = block->next;
	}
	if (block->next) {
		block->next->prev = block->prev;
	}
	if (block->prev) {
		block->prev->next = block->next;
	}
}

// Formats a raw chunk of memory into a valid free block with headers/footers
static void initialize_chunk(void *start, size_t size) {
	block_header_t *block = (block_header_t *) start;
	block->size = size - sizeof(block_header_t) - sizeof(block_footer_t);

	block_footer_t *footer = get_footer(block);
	footer->size = block->size;

	atf(block);
}

void malloc_init(vmem_provider_t provider, void *initial_heap_base, size_t initial_size) {
	raw_vmem_provider = provider;
	heap_end_addr = (uintptr_t) initial_heap_base + initial_size;
	initialize_chunk(initial_heap_base, initial_size);
}

void *malloc(size_t size) {
	if (size == 0) return NULL;

	size_t adjusted_size = ALIGN(size);
	block_header_t *current = free_list_head;

	// First-fit search
	while (current != NULL) {
		if (GET_SIZE(current) >= adjusted_size) {
			break;
		}
		current = current->next;
	}

	// If no block found, expand the heap using VMA
	if (current == NULL) {
		if (!raw_vmem_provider) return NULL;

		size_t total_needed = adjusted_size + sizeof(block_header_t) + sizeof(block_footer_t);
		size_t pages_to_alloc = (total_needed + PAGE_SIZE - 1) / PAGE_SIZE;

		void *new_vmem = raw_vmem_provider(pages_to_alloc);
		if (!new_vmem) return NULL; // Out of memory

		initialize_chunk(new_vmem, pages_to_alloc * PAGE_SIZE);
		heap_end_addr += pages_to_alloc * PAGE_SIZE;

		return malloc(size); // Retry allocation with expanded space
	}

	rff(current);
	size_t remainder = GET_SIZE(current) - adjusted_size;

	// Split block if the remaining space is large enough to hold metadata
	if (remainder >= (sizeof(block_header_t) + sizeof(block_footer_t) + ALIGNMENT)) {
		current->size = adjusted_size;
		get_footer(current)->size = adjusted_size;
		SET_ALLOCATED(current);

		block_header_t *next_block = (block_header_t *) ((uintptr_t) get_footer(current) + sizeof(block_footer_t));
		next_block->size = remainder - sizeof(block_header_t) - sizeof(block_footer_t);

		block_footer_t *next_footer = (block_footer_t *) ((uintptr_t) next_block + sizeof(block_header_t) + GET_SIZE(next_block));
		next_footer->size = GET_SIZE(next_block);

		atf(next_block);
	} else {
		SET_ALLOCATED(current);
	}

	return (void *) ((uintptr_t) current + sizeof(block_header_t));
}

void free(void *ptr) {
	if (!ptr) return;

	block_header_t *block = (block_header_t *) ((uintptr_t) ptr - sizeof(block_header_t));
	atf(block);

	block_header_t *next = get_next_phys(block);
	block_header_t *prev = get_prev_phys(block, (uintptr_t) free_list_head); // Rough approximation of base

	if (next && IS_FREE(next)) {
		rff(next);
		block->size += GET_SIZE(next) + sizeof(block_header_t) + sizeof(block_footer_t);
		get_footer(block)->size = GET_SIZE(block);
	}

	if (prev && IS_FREE(prev)) {
		rff(block);
		prev->size += GET_SIZE(block) + sizeof(block_header_t) + sizeof(block_footer_t);
		get_footer(prev)->size = GET_SIZE(prev);
	}
}

void *calloc(size_t n, size_t size) {
	size_t *new;

	new = malloc(n * size);

	if (new) {
		size_t s = (((n * size) + 3) & ~3) << 2;

		for (size_t i = 0; i < s; i++) {
			new[i] = 0;
		}
	}

	return new;
}