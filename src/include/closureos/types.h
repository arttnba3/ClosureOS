/* SPDX-License-Identifier: GPL-2.0 */

#ifndef CLOSUREOS_TYPES_H
#define CLOSUREOS_TYPES_H

typedef unsigned char       uint8_t;
typedef signed char         int8_t;

typedef unsigned short      uint16_t;
typedef signed short        int16_t;

typedef unsigned int        uint32_t;
typedef signed int          int32_t;

typedef unsigned long long  uint64_t;
typedef signed long long    int64_t;

typedef unsigned long       size_t;
typedef signed long         ssize_t;

typedef int8_t      __s8;
typedef uint8_t     __u8;

typedef int16_t     __s16;
typedef uint16_t    __u16;

typedef int32_t     __s32;
typedef uint32_t    __u32;

typedef int64_t     __s64;
typedef uint64_t    __u64;

typedef __s8    s8;
typedef __u8    u8;

typedef __s16   s16;
typedef __u16   u16;

typedef __s32   s32;
typedef __u32   u32;

typedef __s64   s64;
typedef __u64   u64;

#ifndef NULL
    #define NULL ((void*)0)
#endif

#ifndef __cplusplus

#ifndef true
    #define true 1
#endif

#ifndef false
    #define false 0
#endif

#ifndef bool
    #define bool uint8_t
#endif

#endif // __cplusplus

#endif // CLOSUREOS_TYPES_H
