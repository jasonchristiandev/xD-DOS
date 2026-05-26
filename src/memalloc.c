#include "xddos/memalloc.h"
#include "xddos/memory.h"
#include <string.h>

#define ALIGNMENT 16
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define PAGE_SIZE 4096

#define BLOCK_FREE_BIT ((size_t) 1)
#define GET_SIZE(block) ((block)->size & ~BLOCK_FREE_BIT)
#define IS_FREE(block) ((block)->size & BLOCK_FREE_BIT)
#define GET_SIZE_FROM_FOOTER(footer) ((footer)->size & ~BLOCK_FREE_BIT)
#define IS_FREE_BY_FOOTER(footer) ((footer)->size & BLOCK_FREE_BIT)

typedef struct memalloc_block {
	size_t size;
	struct memalloc_block *next;
	struct memalloc_block *prev;
} memalloc_block_t;

typedef struct block_footer {
	size_t size;
} block_footer_t;

static memalloc_block_t *free_list_head = NULL;
static vmem_provider raw_vmem_provider = NULL;
static uintptr_t heap_start_addr = 0;
static uintptr_t heap_end_addr = 0;

static inline block_footer_t *get_footer(memalloc_block_t *block) {
	return (block_footer_t *) ((uintptr_t) block + sizeof(memalloc_block_t) + GET_SIZE(block));
}

static inline void set_block_state(memalloc_block_t *block, size_t size, uint8_t is_free) {
	if (is_free) {
		block->size = size | BLOCK_FREE_BIT;
	} else {
		block->size = size & ~BLOCK_FREE_BIT;
	}
	get_footer(block)->size = block->size;
}

static inline memalloc_block_t *get_next_phys(memalloc_block_t *block) {
	uintptr_t next_addr = (uintptr_t) get_footer(block) + sizeof(block_footer_t);
	if (next_addr >= heap_end_addr) return NULL;
	return (memalloc_block_t *) next_addr;
}

static void add_to_free(memalloc_block_t *block) {
	block->next = free_list_head;
	block->prev = NULL;
	if (free_list_head) {
		free_list_head->prev = block;
	}
	free_list_head = block;
}

static void remove_from_free(memalloc_block_t *block) {
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

static void initialize_chunk(void *start, size_t size) {
	memalloc_block_t *block = (memalloc_block_t *) start;
	size_t payload_size = size - sizeof(memalloc_block_t) - sizeof(block_footer_t);

	set_block_state(block, payload_size, 1);
	add_to_free(block);
}

void memalloc_init(vmem_provider provider, void *initial_heap_base, size_t initial_size) {
	raw_vmem_provider = provider;
	heap_start_addr = (uintptr_t) initial_heap_base;
	heap_end_addr = heap_start_addr + initial_size;
	initialize_chunk(initial_heap_base, initial_size);
}

void *memalloc_malloc(size_t size) {
	if (size == 0) return NULL;

	size_t adjusted_size = ALIGN(size);
	memalloc_block_t *current = free_list_head;

	while (current != NULL) {
		if (GET_SIZE(current) >= adjusted_size) {
			break;
		}
		current = current->next;
	}

	if (current == NULL) {
		if (!raw_vmem_provider) return NULL;

		size_t total_needed = adjusted_size + sizeof(memalloc_block_t) + sizeof(block_footer_t);
		size_t pages_to_alloc = (total_needed + PAGE_SIZE - 1) / PAGE_SIZE;
		size_t bytes_to_alloc = pages_to_alloc * PAGE_SIZE;

		void *new_vmem = raw_vmem_provider(pages_to_alloc);
		if (!new_vmem) return NULL;

		initialize_chunk(new_vmem, bytes_to_alloc);
		heap_end_addr += bytes_to_alloc;

		return memalloc_malloc(size);
	}

	remove_from_free(current);
	size_t current_size = GET_SIZE(current);
	size_t remainder = current_size - adjusted_size;
	size_t min_block = sizeof(memalloc_block_t) + sizeof(block_footer_t) + ALIGNMENT;

	if (remainder >= min_block) {
		// Mark current as allocated
		set_block_state(current, adjusted_size, 0);

		// Setup the split block
		memalloc_block_t *next_block = (memalloc_block_t *) ((uintptr_t) get_footer(current) + sizeof(block_footer_t));
		size_t next_payload_size = remainder - sizeof(memalloc_block_t) - sizeof(block_footer_t);

		set_block_state(next_block, next_payload_size, 1);
		add_to_free(next_block);
	} else {
		// Allocate the whole thing
		set_block_state(current, current_size, 0);
	}

	return (void *) ((uintptr_t) current + sizeof(memalloc_block_t));
}

void memalloc_free(void *ptr) {
	if (!ptr) return;

	memalloc_block_t *block = (memalloc_block_t *) ((uintptr_t) ptr - sizeof(memalloc_block_t));
	size_t new_size = GET_SIZE(block);

	memalloc_block_t *next = get_next_phys(block);
	if (next && IS_FREE(next)) {
		remove_from_free(next);
		new_size += GET_SIZE(next) + sizeof(memalloc_block_t) + sizeof(block_footer_t);
	}

	if ((uintptr_t) block > heap_start_addr) {
		block_footer_t *prev_footer = (block_footer_t *) ((uintptr_t) block - sizeof(block_footer_t));
		if (IS_FREE_BY_FOOTER(prev_footer)) {
			memalloc_block_t *prev = (memalloc_block_t *) ((uintptr_t) prev_footer - GET_SIZE_FROM_FOOTER(prev_footer) - sizeof(memalloc_block_t));

			remove_from_free(prev);
			new_size += GET_SIZE_FROM_FOOTER(prev_footer) + sizeof(memalloc_block_t) + sizeof(block_footer_t);
			block = prev;
		}
	}

	set_block_state(block, new_size, 1);
	add_to_free(block);
}

void *memalloc_calloc(size_t n, size_t size) {
	size_t total = n * size;
	void *new_ptr = memalloc_malloc(total);

	if (new_ptr) {
		memory_set(new_ptr, 0, total);
	}

	return new_ptr;
}