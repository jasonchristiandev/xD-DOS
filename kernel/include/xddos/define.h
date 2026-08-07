#if defined(__clang__)
#define NO_OPTIMIZE __attribute__((optnone))
#elif defined(__GNUC__)
#define NO_OPTIMIZE __attribute__((optimize("O0")))
#else
#define NO_OPTIMIZE
#endif
#ifndef VERBOSE
#define VERBOSE 0
#endif