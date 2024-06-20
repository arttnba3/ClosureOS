#include <closureos/types.h>

void *boot_memset(void *dst, uint8_t val, uint64_t sz)
{
    uint8_t *__dst = dst;

    for (uint64_t i = 0; i < sz; i++) {
        __dst[i] = val;
    }

    return dst;
}
