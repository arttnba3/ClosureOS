/**
 * Boot stage string library.
 * 
 * Copyright (c) 2024 arttnba3 <arttnba@gmail.com>
 * 
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
*/

#include <closureos/types.h>

void *boot_memset(void *dst, uint8_t val, uint64_t sz)
{
    uint8_t *__dst = dst;

    for (uint64_t i = 0; i < sz; i++) {
        __dst[i] = val;
    }

    return dst;
}
