#ifndef CPUID_H
#define CPUID_H

#include <stdbool.h>

bool cpuid_msr();
bool cpuid_apic();

#endif // !CPUID_H