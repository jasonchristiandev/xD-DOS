#ifndef PMM_H
#define PMM_H

#include <stddef.h>
#include <stdint.h>

#define PAGE_SIZE 4096

int pmm_init(void);
void* pmm_alloc_page(void);
void pmm_free_page(void *page);

#endif // !PMM_H