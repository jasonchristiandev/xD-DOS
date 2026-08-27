#ifndef __XDDOS_ASM_H
#define __XDDOS_ASM_H

#include <stdint.h>

void hlt();
void outl(uint16_t port, uint32_t val);
uint32_t inl(uint16_t port);
void outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);
void outw(uint16_t port, uint16_t val);
uint16_t inw(uint16_t port);
void insw(uint16_t port, void *addr, uint32_t count);
void io_wait();
void invlpg(uint64_t addr);
uint64_t rdmsr(uint32_t msr);
void wrmsr(uint32_t msr, uint64_t value);

#endif // !__XDDOS_ASM_H