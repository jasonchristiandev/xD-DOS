#ifndef __XDDOS_CPUID_H
#define __XDDOS_CPUID_H

#include <stdbool.h>

bool cpuid_msr();
bool cpuid_apic();

#endif // !__XDDOS_CPUID_H