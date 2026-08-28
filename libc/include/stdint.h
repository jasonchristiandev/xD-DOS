#ifndef STDINT_H
#define STDINT_H

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int int16_t;
typedef unsigned short int uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
#if __WORDSIZE == 64
typedef signed long int int64_t;
typedef unsigned long int uint64_t;
#else
typedef signed long long int int64_t;
typedef unsigned long long int uint64_t;
#endif
typedef __INTPTR_TYPE__ intptr_t;
typedef __UINTPTR_TYPE__ uintptr_t;

#if __WORDSIZE == 64
#define SIZE_MAX (18446744073709551615UL)
#else
#if __WORDSIZE32_SIZE_ULONG
#define SIZE_MAX (4294967295UL)
#else
#define SIZE_MAX (4294967295U)
#endif
#endif

#endif // !STDINT_H