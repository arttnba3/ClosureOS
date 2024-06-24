#ifndef CLOSUREOS_LOCK_H
#define CLOSUREOS_LOCK_H

#include <closureos/types.h>
#include <closureos/atomic.h>
#include <closureos/compiler.h>

#define SPINLOCK_LOCKED 1
#define SPINLOCK_FREE 0

typedef struct {
    atomic_t counter;
} spinlock_t;

static __always_inline void spin_lock_init(spinlock_t *lock)
{
    atomic_set(&lock->counter, SPINLOCK_FREE);
}

static __always_inline void spin_lock(spinlock_t *lock)
{
    while (!atomic_compare_and_swap(
        &lock->counter, SPINLOCK_FREE, SPINLOCK_LOCKED
    )) {
        continue;
    }
}

static __always_inline void spin_unlock(spinlock_t *lock)
{
    while (!atomic_compare_and_swap(
        &lock->counter, SPINLOCK_LOCKED, SPINLOCK_FREE
    )) {
        continue;
    }
}

#endif // CLOSUREOS_LOCK_H
