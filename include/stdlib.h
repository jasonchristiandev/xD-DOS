#ifndef STDLIB_H
#define STDLIB_H

#include "xddos/memalloc.h" // IWYU pragma: keep

#define malloc xddos_memalloc_malloc
#define free xddos_memalloc_free
#define calloc xddos_memalloc_calloc

#endif // !STDLIB_H