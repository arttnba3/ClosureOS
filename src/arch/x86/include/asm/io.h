/**
 * Basic operations to interact with IO ports
*/

#ifndef X86_ASM_IO_H
#define X86_ASM_IO_H

#include <closureos/types.h>

static inline __attribute__((always_inline)) void outb(int port, uint8_t value)
{
    asm volatile("outb %0, %w1" : : "a" (value), "d" (port));
}

static inline __attribute__((always_inline)) void outw(int port, uint16_t value)
{
    asm volatile("outw %0, %w1" : : "a" (value), "d" (port));
}

static inline __attribute__((always_inline)) void outl(int port, uint32_t value)
{
    asm volatile("outl %0, %w1" : : "a" (value), "d" (port));
}

static inline __attribute__((always_inline)) uint8_t inb(int port)
{
    uint8_t value;
    asm volatile("inb %w1, %0" : "=a" (value) : "d" (port));
    return value;
}

static inline __attribute__((always_inline)) uint16_t inw(int port)
{
    uint16_t value;
    asm volatile("inw %w1, %0" : "=a" (value) : "d" (port));
    return value;
}

static inline __attribute__((always_inline)) uint32_t inl(int port)
{
    uint32_t value;
    asm volatile("inl %w1, %0" : "=a" (value) : "d" (port));
    return value;
}

#endif // X86_ASM_IO_H
