#ifndef CLOSUREOS_COMPILER_H
#define CLOSUREOS_COMPILER_H

#define __always_inline inline __attribute__((always_inline))

#define barrier() __asm__ __volatile__("": : :"memory")

#endif // CLOSUREOS_COMPILER_H
