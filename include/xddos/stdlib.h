#ifndef STD_H
#define STD_H

#include "xddos/memory.h" // IWYU pragma: keep
#include "xddos/memalloc.h" // IWYU pragma: keep

#define memcpy memory_copy
#define memset memory_set
#define memmove memory_move
#define memcmp memory_cmp
#define malloc memalloc_malloc
#define free memalloc_free
#define calloc memalloc_calloc

#endif // !STD_H