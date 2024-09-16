#ifndef CLOSUREOS_COMPILER_H
#define CLOSUREOS_COMPILER_H

#define __always_inline inline __attribute__((always_inline))

#ifndef barrier
    #define barrier() __asm__ __volatile__("": : :"memory")
#endif
#endif // CLOSUREOS_COMPILER_H
