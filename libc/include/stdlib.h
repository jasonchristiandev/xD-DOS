#ifndef STDLIB_H
#define STDLIB_H

#include "xddos/vma.h" // IWYU pragma: keep

#define malloc vma_malloc
#define free vma_free
#define calloc vma_calloc

#endif // !STDLIB_H