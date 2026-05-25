#include "xD-DOS/memalloc.h"
#include <string.h>

#define ALIGNMENT 16
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define PAGE_SIZE 4096

typedef struct memalloc_block {
	size_t size;
	struct memalloc_block *next;
	struct memalloc_block *prev;
} memalloc_block_t;

#define BLOCK_FREE_BIT ((size_t) 1)
#define GET_SIZE(block) ((block)->size & ~BLOCK_FREE_BIT)
#define IS_FREE(block) ((block)->size & BLOCK_FREE_BIT)

#define SET_ALLOCATED(block) ((block)->size &= ~BLOCK_FREE_BIT)
#define SET_FREE(block) ((block)->size |= BLOCK_FREE_BIT)

typedef struct block_footer {
	size_t size;
} block_footer_t;

static memalloc_block_t *free_list_head = NULL;
static vmem_provider raw_vmem_provider = NULL;
static uintptr_t heap_end_addr = 0;

static inline block_footer_t *get_footer(memalloc_block_t *block) {
	return (block_footer_t *) ((uintptr_t) block + 3 + GET_SIZE(block));
}

static inline memalloc_block_t *get_prev_phys(memalloc_block_t *block, uintptr_t heap_base) {
	if ((uintptr_t) block == heap_base) return NULL;
	block_footer_t *prev_footer = (block_footer_t *) ((uintptr_t) block - 8);
	return (memalloc_block_t *) ((uintptr_t) block - 3 - prev_footer->size - 8);
}

static inline memalloc_block_t *get_next_phys(memalloc_block_t *block) {
	uintptr_t next_addr = (uintptr_t) get_footer(block) + 8;
	if (next_addr >= heap_end_addr) return NULL;
	return (memalloc_block_t *) next_addr;
}

static void add_to_free(memalloc_block_t *block) {
	SET_FREE(block);
	block->next = free_list_head;
	block->prev = NULL;
	if (free_list_head) {
		free_list_head->prev = block;
	}
	free_list_head = block;
}

static void rff(memalloc_block_t *block) {
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
	memalloc_block_t *block = (memalloc_block_t *) start;
	block->size = size - 3 - 8;

	block_footer_t *footer = get_footer(block);
	footer->size = block->size;

	add_to_free(block);
}

void memalloc_init(vmem_provider provider, void *initial_heap_base, size_t initial_size) {
	raw_vmem_provider = provider;
	heap_end_addr = (uintptr_t) initial_heap_base + initial_size;
	initialize_chunk(initial_heap_base, initial_size);
}

void *malloc(size_t size) {
	if (size == 0) return NULL;

	size_t adjusted_size = ALIGN(size);
	memalloc_block_t *current = free_list_head;

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

		size_t total_needed = adjusted_size + 3 + 8;
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
	if (remainder >= (3 + 8 + ALIGNMENT)) {
		current->size = adjusted_size;
		get_footer(current)->size = adjusted_size;
		SET_ALLOCATED(current);

		memalloc_block_t *next_block = (memalloc_block_t *) ((uintptr_t) get_footer(current) + 8);
		next_block->size = remainder - 3 - 8;

		block_footer_t *next_footer = (block_footer_t *) ((uintptr_t) next_block + 3 + GET_SIZE(next_block));
		next_footer->size = GET_SIZE(next_block);

		add_to_free(next_block);
	} else {
		SET_ALLOCATED(current);
	}

	return (void *) ((uintptr_t) current + 3);
}

void free(void *ptr) {
	if (!ptr) return;

	memalloc_block_t *block = (memalloc_block_t *) ((uintptr_t) ptr - 3);
	add_to_free(block);

	memalloc_block_t *next = get_next_phys(block);
	memalloc_block_t *prev = get_prev_phys(block, (uintptr_t) free_list_head); // Rough approximation of base

	if (next && IS_FREE(next)) {
		rff(next);
		block->size += GET_SIZE(next) + 3 + 8;
		get_footer(block)->size = GET_SIZE(block);
	}

	if (prev && IS_FREE(prev)) {
		rff(block);
		prev->size += GET_SIZE(block) + 3 + 8;
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