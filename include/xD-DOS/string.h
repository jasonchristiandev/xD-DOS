#ifndef STRING_H
#define STRING_H

#include <stddef.h>

size_t strlen(const char *str) {
	const char *s;
	
	for (s = str; *s; ++s);
	return (s - str);
}

#endif // !STRING_H