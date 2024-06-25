#ifndef CLOSUREOS_ATOMIC_H
#define CLOSUREOS_ATOMIC_H

#include <closureos/types.h>

typedef volatile int32_t atomic_t;

/**
 * if *ptr == oldval, it'll be set to newval, and the return val will be true,
 * otherwise it'll return false and nothing got changed
 */
#define atomic_compare_and_swap(ptr, oldval, newval) \
    __sync_bool_compare_and_swap(ptr, oldval, newval)

#define atomic_set(ptr, newval) __sync_lock_test_and_set(ptr, newval)

#define atomic_inc(ptr) __sync_fetch_and_add(ptr, 1)
#define atomic_dec(ptr) __sync_fetch_and_sub(ptr, 1)

#define atomic_read(ptr) __sync_fetch_and_add(ptr, 0)

#endif // CLOSUREOS_ATOMIC_H
