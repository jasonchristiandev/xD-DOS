#ifndef STDLIB_H
#define STDLIB_H

#include "xddos/vma.h" // IWYU pragma: keep

#define malloc xddos_vma_malloc
#define free xddos_vma_free
#define calloc xddos_vma_calloc

#endif // !STDLIB_H